#include "utils.h"

PairOfPoints getMinDistance(const Point *points, const int n) {
  double min_d = distance(points[0], points[1]) * 2;
  PairOfPoints result;

  double d;
  for (int i = 0; i < (n - 1); i++) {
    for (int j = i + 1; j < n; j++) {
      d = distance(points[i], points[j]);
      if (d < min_d) {
        min_d = d;
        result.point1 = points[i];
        result.point2 = points[j];
      }
    }
  }

  result.distance = min_d;
  return result;
}

PairOfPoints closestPoints(const Point *points, const int n) {
  // Base case
  if (n <= 3) {
    return getMinDistance(points, n);
  }

  int mid = n / 2;
  Point mid_point = points[mid];

  // calculate left and right minimum distances
  PairOfPoints dl = closestPoints(points, mid);
  PairOfPoints dr = closestPoints(points + mid, n - mid);

  PairOfPoints d = dl.distance < dr.distance ? dl : dr;

  // TODO: Check points with distance from middle < d
}
