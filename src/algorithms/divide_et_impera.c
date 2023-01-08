#ifndef DIVIDE
#define DIVIDE
#include "../utils/points.h"
#include "naive.c"
#include <stdlib.h>

// Declaring closest_points_rec now since it and closest_points_divide are
// mutually recursive

PairOfPoints closest_points_rec(const PointVec sorted_x);

void band_update_result(PointVec band, PairOfPoints *result) {
  Point *points_in_the_band = band.points;
  int points_in_the_band_count = band.length;
  // Sort the points in the band accoring to the y coordinate
  qsort(points_in_the_band, points_in_the_band_count, sizeof(Point), compareY);
  int i, j;
  // Compare their distances with the minimum distance
  for (i = 0; i < points_in_the_band_count; i++) {
    // 7 because you can have at maximum other 6 points in the region
    for (j = 0; j < 6; j++) {
      int k = i + 1 + j;
      if (k < points_in_the_band_count) {
        double distance_now =
            distance(points_in_the_band[i], points_in_the_band[k]);
        if (distance_now < result->distance) {
          result->distance = distance_now;
          result->point1 = points_in_the_band[i];
          result->point2 = points_in_the_band[k];
        }
      }
    }
  }
}

// Returns the closest pair of points using the divide et impera method.
// The input points MUST be sorted by ascending X coordinate
static inline PairOfPoints closest_points_divide(const PointVec sorted_x) {
  // Split the points in half along the X axis
  int half = sorted_x.length / 2;
  int remaining_length = sorted_x.length - half;
  double middle_point =
      (sorted_x.points[half - 1].x + sorted_x.points[half].x + 0.0) / 2.0;
  PointVec left_half;
  left_half.length = half;
  left_half.points = sorted_x.points;
  PointVec right_half;
  right_half.length = remaining_length;
  right_half.points = &sorted_x.points[half];
  // Get the closest ones of each half
  PairOfPoints closest_left = closest_points_rec(left_half);
  PairOfPoints closest_right = closest_points_rec(right_half);
  PairOfPoints result = closest_left.distance < closest_right.distance
                            ? closest_left
                            : closest_right;
  // Calculate delta and the width of the band of points to compare next
  double delta = result.distance;
  int points_in_the_band_count = 0;
  double left_band_limit = middle_point - delta;
  double right_band_limit = middle_point + delta;
  // Populate the band of points
  int i;
  for (i = 0; i < sorted_x.length; i++) {
    int x_coordinate = sorted_x.points[i].x;
    if (x_coordinate < left_band_limit) {
      continue;
    } else if (x_coordinate > right_band_limit) {
      break;
    } else {
      points_in_the_band_count++;
    }
  }
  Point *points_in_the_band =
      (Point *)malloc(sizeof(Point) * points_in_the_band_count);
  int j = 0;
  for (i = 0; i < sorted_x.length; i++) {
    int x_coordinate = sorted_x.points[i].x;
    if (x_coordinate < left_band_limit) {
      continue;
    } else if (x_coordinate > right_band_limit) {
      break;
    } else {
      points_in_the_band[j] = sorted_x.points[i];
      j++;
    }
  }

  PointVec band = {points_in_the_band_count, points_in_the_band};

  band_update_result(band, &result);

  free(points_in_the_band);

  return result;
}

// Finds the closest pair of points among the given ones.
// The input points MUST be sorted by ascending x coordinate
PairOfPoints closest_points_rec(const PointVec sorted_x) {
  if (sorted_x.length == 2) {
    // Base case #1. With two points, we return their distance
    return points_to_pair(sorted_x.points[0], sorted_x.points[1]);
  } else if (sorted_x.length == 3) {
    // Base case #2. With three points, we calculate the closest one
    return closest_points_bruteforce(sorted_x);
  } else {
    return closest_points_divide(sorted_x);
  }
}

PairOfPoints closest_points(PointVec points) {
  // Sort the points
  qsort(points.points, points.length, sizeof(Point), compareX);
  // Find the closest pair
  PairOfPoints result = closest_points_rec(points);
  // Make it so that point 1 is the leftmost of the pair,
  // or the lower one if the x is the same
  if (result.point1.x > result.point2.x) {
    Point tmp = result.point1;
    result.point1 = result.point2;
    result.point2 = tmp;
  } else if (result.point1.x == result.point2.x &&
             result.point1.y > result.point2.y) {
    Point tmp = result.point1;
    result.point1 = result.point2;
    result.point2 = tmp;
  }
  return result;
}

#endif
