#ifndef NAIVE
#define NAIVE
#include "../parallel/utils/points.h"
#include <float.h>

// Returns the closest points in the given vector. Time complexity: (n^2)/2
PairOfPoints closest_points_bruteforce(const PointVec input_points) {
  PairOfPoints result;
  double min_distance = DBL_MAX;
  for (int i = 0; i < input_points.length; i++) {
    for (int j = i + 1; j < input_points.length; j++) {
      const Point first = input_points.points[i];
      const Point second = input_points.points[j];
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
}

#endif
