#include "utils.h"
#include <stdlib.h>

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

PairOfPoints getMinDistanceMidY(Point *points, const int size,
                                const PairOfPoints dist) {
  PairOfPoints min_d = dist;

  qsort(points, size, sizeof(Point), compareY);

  for (int i = 0; i < size; ++i) {
    for (int j = i + 1;
         j < size && (points[j].y - points[i].x) < min_d.distance; j++) {
      if (distance(points[i], points[j]) < min_d.distance) {
        min_d.distance = distance(points[i], points[j]);
        min_d.point1 = points[i];
        min_d.point2 = points[j];
      }
    }
  }
}

PairOfPoints detClosestPoints(const Point *points, const int n) {
  // Base case
  if (n <= 3) {
    return getMinDistance(points, n);
  }

  int mid = n / 2;
  Point mid_point = points[mid];

  // calculate left and right minimum distances
  PairOfPoints dl = detClosestPoints(points, mid);
  PairOfPoints dr = detClosestPoints(points + mid, n - mid);

  PairOfPoints d = dl.distance < dr.distance ? dl : dr;

  // TODO: Check points with distance from middle < d
  Point mid_set[n];
  int k = 0;

  for (int i = 0; i < n; i++) {
    if (distance(points[i], mid_point) < d.distance) {
      mid_set[k] = points[i];
      k++;
    }
  }

  PairOfPoints d_mid = getMinDistanceMidY(mid_set, k, d);

  return d.distance < d_mid.distance ? d : d_mid;
}
