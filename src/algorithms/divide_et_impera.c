#ifndef DIVIDE
#define DIVIDE
#include "../utils/points.h"
#include "naive.c"
#include <stdlib.h>

// Declaring closest_points_rec now since it and closest_points_divide are
// mutually recursive

PairOfPoints closest_points_rec(const PointVec sorted_x);

void band_update_result(PointVec band, double middle_point,
                        PairOfPoints *result) {
  Point *points_in_the_band = band.points;
  int points_in_the_band_count = band.length;
  // Sort the points in the band accoring to the y coordinate
  qsort(points_in_the_band, points_in_the_band_count, sizeof(Point), compareY);
  int i, j;
  // For each point (on the left)
  for (i = 0; i < points_in_the_band_count; i++) {
    Point p = points_in_the_band[i];
    if (p.x > middle_point) {
      continue;
    }
    // Consider the points below it, on the right
    int back = 0;
    for (j = 0; j < 2; j++) {
      int idx = i - 1 - j;
      if (idx < 0) {
        break;
      }
      Point q = points_in_the_band[idx];
      if (q.x < middle_point) {
        j--;
        back++;
        if (back == 3) {
          break;
        }
        continue;
      }
      int diff = p.y - q.y;
      if (diff > result->distance) {
        break;
      }
      double d = distance(p, q);
      if (d < result->distance) {
        result->distance = d;
        result->point1 = p;
        result->point2 = q;
      }
    }
    // Consider the points above it, on the right
    int forward = 0;
    for (j = 0; j < 2; j++) {
      int idx = i + 1 + j;
      if (idx >= points_in_the_band_count) {
        break;
      }
      Point q = points_in_the_band[idx];
      if (q.x < middle_point) {
        j--;
        forward++;
        if (forward == 3) {
          break;
        }
        continue;
      }
      int diff = q.y - p.y;
      if (diff > result->distance) {
        break;
      }
      double d = distance(p, q);
      if (d < result->distance) {
        result->distance = d;
        result->point1 = p;
        result->point2 = q;
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

  band_update_result(band, middle_point, &result);

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
