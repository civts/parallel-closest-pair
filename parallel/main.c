#include "../utils/utils.h"
#include <mpi.h>
#include <stdio.h>

int main() {
  int comm_sz;
  int my_rank;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank != 0) {

    // load data
    debugPrint("Reading the points from file");
    const PointVec point_vec = loadData("../data/5k.txt");

    debugPrint("Sorting the points based on x coordinate");
    qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);

    // split points between processes
    // merge results

  } else {
  }

  MPI_Finalize();
  return 0;
}