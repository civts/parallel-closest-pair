#include "../utils/points_loader.c"
#include "../utils/utils.h"
#include "divide_et_impera.c"
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// // Custom min function used in the reduce
// void minPairOfPoints(PairOfPoints *in, PairOfPoints *out, int *len, MPI_Datatype *dptr) {
//   int i;
//   PairOfPoints min = in[0];

//   printf("len %d\n", *len);

//   for (i = 0; i < *len; i++) {
//     printf("d %.2f\n", in[i].distance);
//     if (in[i].distance < min.distance) {
//       min.distance = in[i].distance;
//       min.point1 = in[i].point1;
//       min.point2 = in[i].point2;
//     }

//     out[i] = min;
//   }
// }

int main(int argc, char **argv) {
  int comm_sz;
  int my_rank;
  int n_points;
  int local_n;
  int i;
  double total_time, read_time;

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

  // MPI initializations
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  total_time = -MPI_Wtime();

  // // Create min operation used in reduce
  // MPI_Op min_op;
  // MPI_Op_create((MPI_User_function*)minPairOfPoints, 1, &min_op);
  MPI_Status mpi_stat;

  // Create Point Datatype for MPI
  MPI_Datatype types[2] = {MPI_INT, MPI_INT};
  MPI_Datatype mpi_point_type;

  MPI_Aint offsets[2];
  offsets[0] = offsetof(Point, x);
  offsets[1] = offsetof(Point, y);
  const int blocklengths[2] = {1, 1};

  MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);

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
  read_time += MPI_Wtime;

  // Broadcast number of points to each process
  MPI_Bcast(&n_points, 1, MPI_INT, 0, MPI_COMM_WORLD);

  fprintf(out_fp, "Received total number of points (%d)\n", n_points);

  int *displs = (int *)malloc(comm_sz * sizeof(int));
  int *scounts = (int *)malloc(comm_sz * sizeof(int));
  Point *borders = (Point *)malloc((comm_sz - 1) * sizeof(Point));

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
    local_d.distance = DBL_MAX;
    fprintf(out_fp, "Less than 2 points: no distance\n");
  }
  
  // Reduce
  // MPI_Reduce(&local_d, &global_d, 1, mpi_point_type, min_op, 0, MPI_COMM_WORLD);

  // TO improve: tree merge
  if (my_rank == 0) {
    global_d = local_d;
    for (i = 1; i < comm_sz; i++) {
      MPI_Recv(&local_d, 1, mpi_point_type, i, i, MPI_COMM_WORLD, &mpi_stat);
      if (local_d.distance < global_d.distance) {
        global_d.distance = local_d.distance;
        global_d.point1 = local_d.point1;
        global_d.point2 = local_d.point2;
      }
    }
  } else {
    MPI_Send(&local_d, 1, mpi_point_type, 0, my_rank, MPI_COMM_WORLD);
  }

  // Check intermediate points
  if (my_rank == 0) {
    fprintf(out_fp, "global_d %.2f, P1 (%d, %d), P2 (%d, %d)\n", global_d.distance, 
            global_d.point1.x, global_d.point1.y,
            global_d.point2.x, global_d.point2.y);
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
  }
  
  // Free memory
  free(displs);
  free(scounts);
  // free(local_points.points);
  // free(point_vec.points);

  total_time += MPI_Wtime();
  if (my_rank == 0) {
    printf("Total time: %f seconds\n", total_time);
    printf("\tReading time: %f seconds\n", read_time);
  }

  MPI_Finalize();
  return 0;
}