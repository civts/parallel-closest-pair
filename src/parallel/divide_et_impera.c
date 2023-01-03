#include "./utils/points.h"
#include <float.h>
#include <stdlib.h>

PairOfPoints getMinDistance(const Point *points, const int n) {
  PairOfPoints result;
  int i, j;
  double d;

  double min_d = distance(points[0], points[1]) * 2;

  for (i = 0; i < (n - 1); i++) {
    for (j = i + 1; j < n; j++) {
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

PairOfPoints getMinDistanceToMidY(Point *points, const int size,
                                  const PairOfPoints dist) {
  int i, j;
  PairOfPoints min_d = dist;

  qsort(points, size, sizeof(Point), compareY);

  for (i = 0; i < size; ++i) {
    for (j = i + 1; j < size && (points[j].y - points[i].y) < min_d.distance;
         j++) {
      if (distance(points[i], points[j]) < min_d.distance) {
        min_d.distance = distance(points[i], points[j]);
        min_d.point1 = points[i];
        min_d.point2 = points[j];
      }
    }
  }

  return min_d;
}

PairOfPoints detClosestPoints(Point *points, const int n) {

  // Base cases
  if (n <= 3) {
    return getMinDistance(points, n);
  }

  PairOfPoints dl, dr, d, d_mid;

  int mid = n / 2;
  Point mid_point = points[mid];

  // Calculate left and right minimum distances
  dl = detClosestPoints(points, mid);
  dr = detClosestPoints(points + mid, n - mid);

  d = dl.distance < dr.distance ? dl : dr;

  // Check points with distance from middle < d
  Point mid_set[n];
  int k = 0;
  int i;

  for (i = 0; i < n; i++) {
    if (distance(points[i], mid_point) < d.distance) {
      mid_set[k] = points[i];
      k++;
    }
  }

  d_mid = getMinDistanceToMidY(mid_set, k, d);

  return d.distance < d_mid.distance ? d : d_mid;
}

PairOfPoints detClosestPointsWrapper(PointVec point_vec) {
  qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);
  return detClosestPoints(point_vec.points, point_vec.length);
}