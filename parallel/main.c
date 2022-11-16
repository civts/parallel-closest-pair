#include "../utils/utils.h"
#include "../utils/points_loader.c"
#include <mpi.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int comm_sz;
  int my_rank;
  int n_points;
  int local_n;

  char *dataset_path = argv[1];

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Create Point Datatype for MPI
  MPI_Datatype types[2] = {MPI_INT, MPI_INT};
  MPI_Datatype mpi_point_type;

  MPI_Aint offsets[2];
  offsets[0] = offsetof(Point, x);
  offsets[1] = offsetof(Point, y);
  const int blocklengths[2] = {1, 1};

  MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);

  printf("Process %d\n", my_rank);

  if (my_rank == 0) {

    // load data
    debugPrint("Reading the points from file");
    const PointVec point_vec = loadData(dataset_path);

    debugPrint("Sorting the points based on x coordinate");
    qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);

    n_points = point_vec.length;
  }

  // Broadcast number of points to each process
  MPI_Bcast(&n_points, 1, MPI_INT, 0, MPI_COMM_WORLD);

  debugPrint("Selecting number of points for each process");
  // If n_points is not a multiple of comm_sz, last process takes the remaining
  // points
  if (my_rank != (comm_sz - 1)) {
    local_n = n_points / comm_sz;
  } else {
    local_n = n_points / comm_sz + (n_points % comm_sz);
  }

  printf("Process %d : %d points\n", my_rank, local_n);

  MPI_Finalize();
  return 0;
}