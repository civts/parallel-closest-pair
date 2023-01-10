#include "./utils/args_parsing.h"
#include "./utils/mpi.h"
#include "./utils/output.h"
#include "./utils/points.h"
#include "./utils/points_loader.h"
#include "./utils/utils.h"
#include "algorithms/divide_et_impera.c"
#include <float.h>
#include <mpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(const int argc, const char *const *const argv) {
  PointVec all_input_points;
  int i;

  // Parse CLI arguments
  print_help_if_needed(argc, argv);
  const char *const dataset_path = parse_dataset_path(argc, argv);
  const char *const output_path = parse_output_path(argc, argv);

  int number_of_processes;
  int my_rank;
  // MPI initializations
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  double total_time = -MPI_Wtime();

  // Create Point Datatype for MPI
  MPI_Datatype mpi_point_type = create_point_datatype();
  MPI_Datatype mpi_pair_of_points_type =
      create_pair_of_points_datatype(mpi_point_type);

  // setup output file
  FILE *out_fp = setup_file(my_rank, output_path);

  double read_time;
  if (my_rank == 0) {
    read_time = -MPI_Wtime();

    // load data
    debugPrint("Reading the points from file");
    all_input_points = loadData(dataset_path);

    read_time += MPI_Wtime();

    debugPrint("Sorting the points based on x coordinate");
    qsort(all_input_points.points, all_input_points.length, sizeof(Point),
          compareX);

    fprintf(out_fp, "Loaded and sorted %d points\n", all_input_points.length);
  }

  // Broadcast number of points to each process
  MPI_Bcast(&all_input_points.length, 1, MPI_INT, 0, MPI_COMM_WORLD);

  fprintf(out_fp, "Received total number of points (%d)\n",
          all_input_points.length);

  double scatter_time = -MPI_Wtime();
  int *displs = (int *)malloc(number_of_processes * sizeof(int));
  int *local_count = (int *)malloc(number_of_processes * sizeof(int));
  check_not_failed_or_exit(displs, "displs");
  check_not_failed_or_exit(local_count, "local_count");

  int stride = all_input_points.length / number_of_processes;

  // If all_input_points.length is not a multiple of comm_sz, last process takes
  // the remaining points
  for (i = 0; i < number_of_processes; ++i) {
    displs[i] = i * stride;
    if (i != (number_of_processes - 1)) {
      local_count[i] = stride;
    } else {
      local_count[i] = stride + (all_input_points.length % number_of_processes);
    }
  }

  int local_n = local_count[my_rank];

  PointVec local_points;
  local_points.length = local_n;
  local_points.points = (Point *)malloc(local_n * sizeof(Point));
  check_not_failed_or_exit(local_points.points, "local_pointspoints");

  fprintf(out_fp, "Local number of points: %d\n", local_n);

  debugPrint("Sending points to each process");

  MPI_Scatterv(all_input_points.points, local_count, displs, mpi_point_type,
               local_points.points, local_n, mpi_point_type, 0, MPI_COMM_WORLD);

  scatter_time += MPI_Wtime();

  fprintf(out_fp, "First point: (%d, %d)\n", local_points.points[0].x,
          local_points.points[0].y);
  fprintf(out_fp, "Last point: (%d, %d)\n",
          local_points.points[local_points.length - 1].x,
          local_points.points[local_points.length - 1].y);

  PairOfPoints local_best;
  if (local_points.length > 1) {
    local_best = closest_points(local_points);
    fprintf(
        out_fp,
        "\nLocal smallest distance: %.2f\nBetween (%d, %d) and (%d, %d)\n\n",
        local_best.distance, local_best.point1.x, local_best.point1.y,
        local_best.point2.x, local_best.point2.y);
  } else {
    local_best.distance = DBL_MAX;
    fprintf(out_fp, "Less than 2 points: no distance\n");
  }

  // Tree merge
  int levels = ceil(log2(number_of_processes));
  int current_level;
  bool already_sent = false;
  for (current_level = 0; current_level < levels; current_level++) {

    if (already_sent) {
      break;
    }

    bool not_process_zero = my_rank != 0;
    bool am_i_sender_on_this_level =
        my_rank % (int)pow(2, current_level + 1) != 0;
    bool am_i_a_sender = am_i_sender_on_this_level && not_process_zero;

    if (am_i_a_sender) {
      int dest = my_rank - (int)pow(2, current_level);

      // Send to processes with rank (my_rank - 2^i)
      MPI_Send(&local_best, 1, mpi_pair_of_points_type, dest, my_rank,
               MPI_COMM_WORLD);
      already_sent = true;

      fprintf(out_fp, "Sending local_best to process %d\n", dest);

      // Finding border points

      // This is not exact, we may fix it later
      Point leftmost_point = local_points.points[0];
      Point rightmost_point = local_points.points[local_points.length - 1];

      // Figure out how many points we have in the bands
      int i;
      int points_in_left_band = 0;
      int points_in_right_band = 0;
      for (i = 0; i < local_points.length; i++) {
        Point current_point = local_points.points[i];
        double d = current_point.x - leftmost_point.x;
        if (d < local_best.distance) {
          points_in_left_band++;
        } else {
          break;
        }
      }
      for (i = local_points.length - 1; i >= 0; i--) {
        Point current_point = local_points.points[i];
        double d = rightmost_point.x - current_point.x;
        if (d < local_best.distance) {
          points_in_right_band++;
        } else {
          break;
        }
      }

      // Allocate and populate the bands
      PointVec left_band_points = {
          points_in_left_band,
          malloc(points_in_left_band * sizeof(Point)),
      };
      check_not_failed_or_exit(left_band_points.points,
                               "left_band_points.points");
      PointVec right_band_points = {
          points_in_right_band,
          malloc(points_in_right_band * sizeof(Point)),
      };
      check_not_failed_or_exit(right_band_points.points,
                               "right_band_points.points");

      int j = 0;
      for (i = 0; i < local_points.length; i++) {
        Point current_point = local_points.points[i];
        double d = current_point.x - leftmost_point.x;
        if (d < local_best.distance) {
          left_band_points.points[j] = current_point;
          j++;
        } else {
          break;
        }
      }
      j = 0;
      for (i = local_points.length - 1; i >= 0; i--) {
        Point current_point = local_points.points[i];
        double d = rightmost_point.x - current_point.x;
        if (d < local_best.distance) {
          right_band_points.points[j] = current_point;
          j++;
        } else {
          break;
        }
      }

      // Send border count
      int count_array[] = {points_in_left_band, points_in_right_band};
      MPI_Send(&count_array, 2, MPI_INT, dest, my_rank, MPI_COMM_WORLD);

      // Send border points
      int total_points_to_send = points_in_left_band + points_in_right_band;
      Point *all_border_points = malloc(sizeof(Point) * (total_points_to_send));
      check_not_failed_or_exit(all_border_points, "all_border_points");
      memcpy(all_border_points, left_band_points.points,
             points_in_left_band * sizeof(Point));
      memcpy(&all_border_points[points_in_left_band], right_band_points.points,
             points_in_right_band * sizeof(Point));

      MPI_Send(all_border_points, total_points_to_send, mpi_point_type, dest,
               my_rank, MPI_COMM_WORLD);

      free(left_band_points.points);
      free(right_band_points.points);
    } else {
      // Receive best distance that the other process found, and merge with our
      // result

      int src = my_rank + (int)pow(2, current_level);
      MPI_Status mpi_stat;
      PairOfPoints partial_best;

      MPI_Recv(&partial_best, 1, mpi_pair_of_points_type, src, src,
               MPI_COMM_WORLD, &mpi_stat);
      fprintf(out_fp,
              "Received local_best from process %d, %.2f, P1 (%d, %d), P2 (%d, "
              "%d)\n",
              src, partial_best.distance, partial_best.point1.x,
              partial_best.point1.y, partial_best.point2.x,
              partial_best.point2.y);

      // Update minimum distance based on the received one
      if (partial_best.distance < local_best.distance) {
        local_best.distance = partial_best.distance;
        local_best.point1 = partial_best.point1;
        local_best.point2 = partial_best.point2;
      }

      fprintf(out_fp,
              "Local_best after updating %.2f, P1 (%d, %d), P2 (%d, "
              "%d)\n",
              local_best.distance, local_best.point1.x, local_best.point1.y,
              local_best.point2.x, local_best.point2.y);

      // Receive border points
      int border_count[2];
      MPI_Recv(&border_count, 2, MPI_INT, src, src, MPI_COMM_WORLD, &mpi_stat);

      int central_left_band_len = border_count[0];
      int right_band_len = border_count[1];

      PointVec central_left_band = {
          central_left_band_len,
          malloc(sizeof(Point) * central_left_band_len),
      };
      check_not_failed_or_exit(central_left_band.points,
                               "central_left_band.points");
      PointVec right_band = {
          right_band_len,
          malloc(sizeof(Point) * right_band_len),
      };
      check_not_failed_or_exit(right_band.points, "right_band.points");
      int total_border_points = right_band_len + central_left_band_len;
      Point *points_to_receive = malloc(sizeof(Point) * total_border_points);
      check_not_failed_or_exit(points_to_receive, "points_to_receive");

      // Receive centrel-left and right points from other process
      MPI_Recv(points_to_receive, total_border_points, mpi_point_type, src, src,
               MPI_COMM_WORLD, &mpi_stat);
      memcpy(central_left_band.points, points_to_receive,
             central_left_band_len * sizeof(Point));
      memcpy(right_band.points, &points_to_receive[central_left_band_len],
             right_band_len * sizeof(Point));
      free(points_to_receive);

      // Populate our central-right band
      int central_right_band_len = 0;
      int central_left_margin_x = central_left_band.points[0].x;
      for (i = local_points.length - 1; i >= 0; i--) {
        Point p = local_points.points[i];
        double distance_now = central_left_margin_x - p.x;
        if (distance_now < local_best.distance) {
          central_right_band_len++;
        } else {
          break;
        }
      }
      int central_band_len = central_right_band_len + central_left_band_len;
      PointVec central_band = {
          central_band_len,
          malloc(sizeof(Point) * central_band_len),
      };
      check_not_failed_or_exit(central_band.points, "central_band.points");
      memcpy(central_band.points,
             &local_points.points[local_points.length - central_right_band_len],
             central_right_band_len * sizeof(Point));

      // Finish populating the central band by adding the central-left points
      memcpy(&central_band.points[central_right_band_len],
             central_left_band.points, central_left_band_len * sizeof(Point));
      free(central_left_band.points);

      // Check the band for closer points (eventually updating local best)
      band_update_result(central_band, &local_best);

      fprintf(out_fp,
              "Local_best after band update %.2f, P1 (%d, %d), P2 (%d, "
              "%d)\n",
              local_best.distance, local_best.point1.x, local_best.point1.y,
              local_best.point2.x, local_best.point2.y);

      free(central_band.points);

      // Compute left band
      int left_band_len = 0;
      int left_margin_x = local_points.points[0].x;
      for (i = 0; i < local_points.length; i++) {
        Point p = local_points.points[i];
        double distance_now = p.x - left_margin_x;
        if (distance_now < local_best.distance) {
          left_band_len++;
        } else {
          break;
        }
      }
      PointVec left_band = {
          left_band_len,
          malloc(left_band_len * sizeof(Point)),
      };
      check_not_failed_or_exit(left_band.points, "left_band.points");
      memcpy(left_band.points, local_points.points,
             left_band_len * sizeof(Point));

      // Filter the right band by removing points beyond the current best
      // distance
      int new_right_band_len = 0;
      int right_margin_x = right_band.points[right_band_len - 1].x;
      for (i = right_band_len - 1; i >= 0; i--) {
        double distance = right_margin_x - right_band.points[i].x;
        if (distance < local_best.distance) {
          new_right_band_len++;
        } else {
          break;
        }
      }
      Point *new_right_band_points = malloc(new_right_band_len * sizeof(Point));
      check_not_failed_or_exit(new_right_band_points, "new_right_band_points");
      memcpy(new_right_band_points,
             &right_band.points[right_band.length - new_right_band_len],
             new_right_band_len * sizeof(Point));
      free(right_band.points);
      right_band.length = new_right_band_len;
      right_band.points = new_right_band_points;

      // Local points can now become just the two bands
      int new_local_points_len = left_band.length + right_band.length;
      Point *new_local_points = malloc(new_local_points_len * sizeof(Point));
      check_not_failed_or_exit(new_local_points, "new_local_points");
      memcpy(new_local_points, left_band.points,
             left_band.length * sizeof(Point));
      memcpy(&new_local_points[left_band.length], right_band.points,
             right_band.length * sizeof(Point));

      free(left_band.points);
      free(right_band.points);
      free(local_points.points);
      local_points.points = new_local_points;
      local_points.length = new_local_points_len;

      fprintf(out_fp, "New local_best: %.2f, P1 (%d, %d), P2 (%d, %d)\n\n",
              local_best.distance, local_best.point1.x, local_best.point1.y,
              local_best.point2.x, local_best.point2.y);
    }
  }

  // Free memory
  free(displs);
  free(local_count);
  free(local_points.points);

  total_time += MPI_Wtime();
  if (my_rank == 0) {
    free(all_input_points.points);
    printf("Final distance: %f P1 (%d, %d)  P2 (%d, %d)\n", local_best.distance,
           local_best.point1.x, local_best.point1.y, local_best.point2.x,
           local_best.point2.y);
    printf("Total time: %f seconds\n", total_time);
    printf("Reading time: %f seconds\n", read_time);
    printf("Scatter time: %f seconds\n", scatter_time);

    fprintf(out_fp, "Final distance: %f P1 (%d, %d)  P2 (%d, %d)\n",
            local_best.distance, local_best.point1.x, local_best.point1.y,
            local_best.point2.x, local_best.point2.y);
    fprintf(out_fp, "Total time: %f seconds\n", total_time);
    fprintf(out_fp, "Reading time: %f seconds\n", read_time);
    fprintf(out_fp, "Scatter time: %f seconds\n", scatter_time);

    FILE *res_fp = setup_file(-1, output_path);
    fprintf(res_fp, "The minimum distance is %f", local_best.distance);
    close_file(res_fp);

    printf("All done!\n");
  }

  close_file(out_fp);

  printf("Process %d completed\n", my_rank);
  MPI_Finalize();
  return 0;
}
