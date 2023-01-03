#include "../parallel/utils/points.h"
#include <math.h>
#include <stdlib.h>

// Finds the closest pair of points among the given ones.
// The input points must be sorted according to their x coordinates.
PairOfPoints closestPointsRec(const PointVec sorted_x) {
  if (sorted_x.length == 2) {
    // Base case #1. With two points, we return their distance
    PairOfPoints result;
    result.point1 = sorted_x.points[0];
    result.point2 = sorted_x.points[1];
    result.distance = distance(result.point1, result.point2);
    return result;
  } else if (sorted_x.length == 3) {
    // Base case #2. With three points, we calculate the closest one
    PairOfPoints result;
    double min_distance = __DBL_MAX__;
    for (int i = 0; i < sorted_x.length; i++) {
      for (int j = i + 1; j > sorted_x.length; j++) {
        const Point first = sorted_x.points[i];
        const Point second = sorted_x.points[j];
        double distance_now = distance(first, second);
        if (distance_now < min_distance) {
          min_distance = distance_now;
          result.point1 = first;
          result.point2 = second;
        }
      }
    }
    result.distance = min_distance;
    return result;
  } else {
    // Split the points in half along the X axis
    int half = sorted_x.length / 2;
    int remaining_length = sorted_x.length - half;
    PointVec left_half;
    left_half.length = half;
    left_half.points = sorted_x.points;
    PointVec right_half;
    right_half.length = remaining_length;
    right_half.points = &sorted_x.points[half];
    // Get the closest ones of each half
    PairOfPoints closest_left = closestPointsRec(left_half);
    PairOfPoints closest_right = closestPointsRec(right_half);
    PairOfPoints result = closest_left.distance < closest_right.distance
                              ? closest_left
                              : closest_right;
    // Calculate delta and the width of the band of points to compare next
    double delta = result.distance;
    double separator_x =
        (sorted_x.points[half - 1].x + sorted_x.points[half].x) / 2;
    int points_in_the_band_count = 0;
    double left_band_limit = separator_x - delta;
    double right_band_limit = separator_x + delta;
    // Populate the band of points
    for (int i = 0; i < sorted_x.length; i++) {
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
    for (int i = 0; i < sorted_x.length; i++) {
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
    // Sort the points in the band accoring to the y coordinate
    qsort(points_in_the_band, points_in_the_band_count, sizeof(Point),
          compareY);
    // Compare their distances with the minimum distance
    for (int i = 0; i < points_in_the_band_count; i++) {
      // 7 because you can have at maximum other 6 points in the region
      for (int j = 0; j < 6; j++) {
        int k = i + 1 + j;
        if (k < points_in_the_band_count) {
          double distance_now =
              distance(points_in_the_band[i], points_in_the_band[k]);
          if (distance_now < result.distance) {
            result.distance = distance_now;
            result.point1 = points_in_the_band[i];
            result.point2 = points_in_the_band[k];
          }
        }
      }
    }
    free(points_in_the_band);
    return result;
  }
}

// Finds the closest pair of points among the given ones.
PairOfPoints closestPoints(PointVec points) {
  // Sort the points
  qsort(points.points, points.length, sizeof(Point), compareX);
  // Find the closest pair
  PairOfPoints result = closestPointsRec(points);
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
