#include "./utils/args_parsing.c"
#include "./utils/finalize.c"
#include "./utils/mpi.h"
#include "./utils/output.h"
#include "./utils/points.h"
#include "./utils/points_loader.c"
#include "divide_et_impera.c"
#include <mpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(const int argc, char *const *const argv) {
  double total_time, read_time, scatter_time;

  PointVec all_input_points;
  PairOfPoints local_d, recv_d;

  // Parse CLI arguments
  print_help_if_needed(argc, argv);
  const char *const dataset_path = parse_dataset_path(argc, argv);
  const char *const output_path = parse_output_path(argc, argv);

  int comm_sz;
  int my_rank;
  // MPI initializations
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  total_time = -MPI_Wtime();

  MPI_Status mpi_stat;

  // Create Point Datatype for MPI
  MPI_Datatype mpi_point_type = create_point_datatype();
  MPI_Datatype mpi_pair_of_points_type =
      create_pair_of_points_datatype(mpi_point_type);

  // setup output file
  const FILE *out_fp = setup_file(my_rank, output_path);

  read_time = -MPI_Wtime();
  if (my_rank == 0) {

    // load data
    debugPrint("Reading the points from file");
    all_input_points = loadData(dataset_path);

    debugPrint("Sorting the points based on x coordinate");
    qsort(all_input_points.points, all_input_points.length, sizeof(Point),
          compareX);

    fprintf(out_fp, "Loaded and sorted %d points\n", all_input_points.length);
  }
  read_time += MPI_Wtime();

  // Broadcast number of points to each process
  MPI_Bcast(&all_input_points.length, 1, MPI_INT, 0, MPI_COMM_WORLD);

  fprintf(out_fp, "Received total number of points (%d)\n",
          all_input_points.length);

  scatter_time = -MPI_Wtime();
  int *displs = (int *)malloc(comm_sz * sizeof(int));
  int *local_count = (int *)malloc(comm_sz * sizeof(int));

  int stride = all_input_points.length / comm_sz;

  // If all_input_points.length is not a multiple of comm_sz, last process takes
  // the remaining points
  for (int i = 0; i < comm_sz; ++i) {
    displs[i] = i * stride;
    if (i != (comm_sz - 1)) {
      local_count[i] = stride;
    } else {
      local_count[i] = stride + (all_input_points.length % comm_sz);
    }
  }

  int local_n = local_count[my_rank];

  PointVec local_points;
  local_points.length = local_n;
  local_points.points = (Point *)malloc(local_n * sizeof(Point));

  fprintf(out_fp, "Local number of points: %d\n", local_n);

  debugPrint("Sending points to each process");

  MPI_Scatterv(all_input_points.points, local_count, displs, mpi_point_type,
               local_points.points, local_n, mpi_point_type, 0, MPI_COMM_WORLD);

  scatter_time += MPI_Wtime();

  for (int i = 0; i < local_points.length; i++) {
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
  int levels = (int)log2(comm_sz);
  for (int i = 0; i < levels; i++) {
    if (my_rank % (int)pow(2, i + 1) != 0 && my_rank != 0) {
      int dest = my_rank - (int)pow(2, i);
      if (dest >= 0) {
        // Send to processes with rank (my_rank - 2^i)
        MPI_Send(&local_d, 1, mpi_pair_of_points_type, dest, my_rank,
                 MPI_COMM_WORLD);

        fprintf(out_fp, "Sending local_d to process %d\n", dest);

        // TODO send border points
      }
    } else {
      // Receive local distance and merge
      int src = my_rank + (int)pow(2, i);
      MPI_Recv(&recv_d, 1, mpi_pair_of_points_type, src, src, MPI_COMM_WORLD,
               &mpi_stat);
      fprintf(
          out_fp,
          "Received local_d from process %d, %.2f, P1 (%d, %d), P2 (%d, %d)\n",
          src, recv_d.distance, recv_d.point1.x, recv_d.point1.y,
          recv_d.point2.x, recv_d.point2.y);

      if (recv_d.distance < local_d.distance) {
        local_d.distance = recv_d.distance;
        local_d.point1 = recv_d.point1;
        local_d.point2 = recv_d.point2;
      }

      fprintf(out_fp, "Local_d: %.2f, P1 (%d, %d), P2 (%d, %d)\n",
              local_d.distance, local_d.point1.x, local_d.point1.y,
              local_d.point2.x, local_d.point2.y);

      // TODO receive and merge border points
    }
  }

  // Free memory
  free(displs);
  free(local_count);
  free(local_points.points);
  // free(point_vec.points);
  close_file(out_fp);

  total_time += MPI_Wtime();
  if (my_rank == 0) {
    printf("Final distance: %f P1 (%d, %d)  P2 (%d, %d)\n", local_d.distance,
           local_d.point1.x, local_d.point1.y, local_d.point2.x,
           local_d.point2.y);
    printf("Total time: %f seconds\n", total_time);
    printf("\tReading time: %f seconds\n", read_time);
    printf("\tScatter time: %f seconds\n", scatter_time);

    char *const finalize_script_path = argv[3];
    if (finalize_script_path != NULL && *finalize_script_path != '\0') {
      call_finalize_script(finalize_script_path);
    }

    printf("Done!\n");
  }

  printf("Process %d   completed\n", my_rank);
  MPI_Finalize();
  return 0;
}
