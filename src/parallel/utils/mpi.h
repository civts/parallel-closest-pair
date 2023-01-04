#include "points.h"
#include <mpi.h>

MPI_Datatype create_point_datatype() {
  MPI_Datatype types_1[2] = {MPI_INT, MPI_INT};
  MPI_Datatype mpi_point_type;

  MPI_Aint offsets_1[2];
  offsets_1[0] = offsetof(Point, x);
  offsets_1[1] = offsetof(Point, y);
  const int blocklengths_1[2] = {1, 1};

  MPI_Type_create_struct(2, blocklengths_1, offsets_1, types_1,
                         &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);

  return mpi_point_type;
}

MPI_Datatype create_pair_of_points_datatype(const MPI_Datatype mpi_point_type) {
  // Create PairOfPoints Datatype for MPI
  MPI_Datatype types_2[3] = {MPI_DOUBLE, mpi_point_type, mpi_point_type};
  MPI_Datatype mpi_pair_of_points_type;

  MPI_Aint offsets_2[3];
  offsets_2[0] = offsetof(PairOfPoints, distance);
  offsets_2[1] = offsetof(PairOfPoints, point1);
  offsets_2[2] = offsetof(PairOfPoints, point2);
  const int blocklengths_2[3] = {1, 1, 1};

  MPI_Type_create_struct(3, blocklengths_2, offsets_2, types_2,
                         &mpi_pair_of_points_type);
  MPI_Type_commit(&mpi_pair_of_points_type);
  return mpi_pair_of_points_type;
}
