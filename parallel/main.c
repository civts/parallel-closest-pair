#include "../utils/points_loader.c"
#include "../utils/utils.h"
#include "divide_et_impera.c"
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int comm_sz;
  int my_rank;
  int n_points;
  int local_n;
  int levels;
  int i;
  double total_time, read_time, scatter_time;

  PointVec point_vec;
  PointVec local_points;
  PairOfPoints local_d;
  PairOfPoints recv_d;
  FILE *out_fp;

  char *dataset_path = argv[1];
  if (dataset_path == NULL || *dataset_path == '\0') {
    printf("Missing positional argument 1: the path to the dataset file. "
           "Terminating\n");
    exit(1);
  }
  char *output_path = argv[2];
  if (output_path == NULL || *output_path == '\0') {
    printf("Missing positional argument 2: the path to the output file. "
           "Terminating\n");
    exit(1);
  }

  // MPI initializations
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  total_time = -MPI_Wtime();

  MPI_Status mpi_stat;

  // Create Point Datatype for MPI
  MPI_Datatype types_1[2] = {MPI_INT, MPI_INT};
  MPI_Datatype mpi_point_type;

  MPI_Aint offsets_1[2];
  offsets_1[0] = offsetof(Point, x);
  offsets_1[1] = offsetof(Point, y);
  const int blocklengths_1[2] = {1, 1};

  MPI_Type_create_struct(2, blocklengths_1, offsets_1, types_1, &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);

  // Create PairOfPoints Datatype for MPI
  MPI_Datatype types_2[3] = {MPI_DOUBLE, mpi_point_type, mpi_point_type};
  MPI_Datatype mpi_pair_of_points_type;

  MPI_Aint offsets_2[3];
  offsets_2[0] = offsetof(PairOfPoints, distance);
  offsets_2[1] = offsetof(PairOfPoints, point1);
  offsets_2[2] = offsetof(PairOfPoints, point2);
  const int blocklengths_2[3] = {1, 1, 1};

  MPI_Type_create_struct(3, blocklengths_2, offsets_2, types_2, &mpi_pair_of_points_type);
  MPI_Type_commit(&mpi_pair_of_points_type);

  // setup output file
  int num_digits = 0;
  int tmp_rank = my_rank;
  do {
    tmp_rank /= 10;
    num_digits++;
  } while (tmp_rank > 0);
  char *rank_str = malloc(sizeof(char) * (num_digits + 1));
  sprintf(rank_str, "%d.txt", my_rank);

  out_fp = fopen(strcat(output_path, rank_str), "w+");
  fprintf(out_fp, "Process %d\n", my_rank);
  free(rank_str);

  read_time = -MPI_Wtime();
  if (my_rank == 0) {

    // load data
    debugPrint("Reading the points from file");
    point_vec = loadData(dataset_path);

    debugPrint("Sorting the points based on x coordinate");
    qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);

    n_points = point_vec.length;

    fprintf(out_fp, "Loaded and sorted %d points\n", n_points);
  }
  read_time += MPI_Wtime();

  // Broadcast number of points to each process
  MPI_Bcast(&n_points, 1, MPI_INT, 0, MPI_COMM_WORLD);

  fprintf(out_fp, "Received total number of points (%d)\n", n_points);

  scatter_time = -MPI_Wtime();
  int *displs = (int *)malloc(comm_sz * sizeof(int));
  int *scounts = (int *)malloc(comm_sz * sizeof(int));

  int stride = n_points / comm_sz;

  // If n_points is not a multiple of comm_sz, last process takes the remaining
  // points
  for (i = 0; i < comm_sz; ++i) {
    displs[i] = i * stride;
    if (i != (comm_sz - 1)) {
      scounts[i] = stride;
    } else {
      scounts[i] = stride + (n_points % comm_sz);
    }
  }

  local_n = scounts[my_rank];

  local_points.length = local_n;
  local_points.points = (Point *)malloc(local_n * sizeof(Point));

  fprintf(out_fp, "Local number of points: %d\n", local_n);

  debugPrint("Sending points to each process");

  MPI_Scatterv(point_vec.points, scounts, displs, mpi_point_type,
               local_points.points, local_n, mpi_point_type, 0, MPI_COMM_WORLD);

  scatter_time += MPI_Wtime();

  for (i = 0; i < local_points.length; i++) {
    fprintf(out_fp, "(%d, %d)\n", local_points.points[i].x,
            local_points.points[i].y);
  }

  if (local_points.length > 1) {
    local_d = detClosestPointsWrapper(local_points);
    fprintf(out_fp, "Local smallest distance: %.2f\n", local_d.distance);
  } else {
    local_d.distance = DBL_MAX;
    fprintf(out_fp, "Less than 2 points: no distance\n");
  }

  // Tree merge
  levels = (int)log2(comm_sz);
  for (i = 0; i < levels; i++) {
    if (my_rank % (int)pow(2, i + 1) != 0 && my_rank != 0) {
      int dest = my_rank - (int)pow(2, i);
      if (dest >= 0) {
        //Send to processes with rank (my_rank - 2^i)
        MPI_Send(&local_d, 1, mpi_pair_of_points_type, dest, my_rank, MPI_COMM_WORLD);

        fprintf(out_fp, "Sending local_d to process %d\n", dest);
        
        // TODO send border points
      }
    } else {
      // Receive local distance and merge
      int src = my_rank + (int)pow(2, i);
      MPI_Recv(&recv_d, 1, mpi_pair_of_points_type, src, src, MPI_COMM_WORLD, &mpi_stat);
      fprintf(out_fp, "Received local_d from process %d, %.2f, P1 (%d, %d), P2 (%d, %d)\n", 
              src,  recv_d.distance, 
              recv_d.point1.x, recv_d.point1.y,
              recv_d.point2.x, recv_d.point2.y);
      
      if (recv_d.distance < local_d.distance) {
        local_d.distance = recv_d.distance;
        local_d.point1 = recv_d.point1;
        local_d.point2 = recv_d.point2;
      }

      fprintf(out_fp, "Local_d: %.2f, P1 (%d, %d), P2 (%d, %d)\n", local_d.distance, 
              local_d.point1.x, local_d.point1.y,
              local_d.point2.x, local_d.point2.y);
      
      // TODO receive and merge border points
    }
  }
  
  // Free memory
  free(displs);
  free(scounts);
  // free(local_points.points);
  // free(point_vec.points);

  total_time += MPI_Wtime();
  if (my_rank == 0) {
    printf("Final distance: %f P1 (%d, %d)  P2 (%d, %d)\n", local_d.distance, 
            local_d.point1.x, local_d.point1.y,
            local_d.point2.x, local_d.point2.y);
    printf("Total time: %f seconds\n", total_time);
    printf("\tReading time: %f seconds\n", read_time);
    printf("\tScatter time: %f seconds\n", scatter_time);
  }

  MPI_Finalize();
  return 0;
}