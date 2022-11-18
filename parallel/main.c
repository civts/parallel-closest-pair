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
  int i;
  PointVec point_vec;
  PointVec local_points;
  PairOfPoints local_d;
  PairOfPoints global_d;
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

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

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

  // Create Point Datatype for MPI
  MPI_Datatype types[2] = {MPI_INT, MPI_INT};
  MPI_Datatype mpi_point_type;

  MPI_Aint offsets[2];
  offsets[0] = offsetof(Point, x);
  offsets[1] = offsetof(Point, y);
  const int blocklengths[2] = {1, 1};

  MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);

  if (my_rank == 0) {

    // load data
    debugPrint("Reading the points from file");
    point_vec = loadData(dataset_path);

    debugPrint("Sorting the points based on x coordinate");
    qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);

    n_points = point_vec.length;

    fprintf(out_fp, "Loaded and sorted %d points\n", n_points);
  }

  // Broadcast number of points to each process
  MPI_Bcast(&n_points, 1, MPI_INT, 0, MPI_COMM_WORLD);

  fprintf(out_fp, "Received total number of points (%d)\n", n_points);

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

  for (i = 0; i < local_points.length; i++) {
    fprintf(out_fp, "(%d, %d)\n", local_points.points[i].x,
            local_points.points[i].y);
  }

  if (local_points.length > 1) {
    local_d = detClosestPointsWrapper(local_points);
    fprintf(out_fp, "Local smallest distance: %.2f\n", local_d.distance);
  } else {
    fprintf(out_fp, "Less than 2 points: no distance\n");
  }

  // TODO: Send local_d to process 0
  // Check intermediate points
  // if (my_rank == 0) {
  //   double dist;
  //   Point p1, p2;

  //   global_d = local_d;
    
  //   for (i = 1; i < comm_sz; i++) {
  //     p1 = point_vec[i*stride-1];
  //     p2 = point_vec[i*stride];
  //     dist = distance(p1, p2);
  //     if (dist < global_d.distance) {
  //       global_d.distance = dist;
  //       global_d.point1 = p1;
  //       global_d.point2 = p2;
  //     }
  //   }
  // }
  
  // Free memory
  free(displs);
  free(scounts);
  free(local_points.points);
  free(point_vec.points);

  MPI_Finalize();
  return 0;
}