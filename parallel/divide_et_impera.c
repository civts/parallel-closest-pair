#include "../utils/utils.h"
#include <float.h>
#include <stdlib.h>

PairOfPoints getMinDistance(PointVec point_vec) {
  PairOfPoints result;
  int i,j;
  double d;

  double min_d = distance(point_vec.points[0], point_vec.points[1]) * 2;

  for (i = 0; i < (point_vec.length - 1); i++) {
    for (j = i + 1; j < point_vec.length; j++) {
      d = distance(point_vec.points[i], point_vec.points[j]);
      if (d < min_d) {
        min_d = d;
        result.point1 = point_vec.points[i];
        result.point2 = point_vec.points[j];
      }
    }
  }

  result.distance = min_d;
  return result;
}

PairOfPoints getMinDistanceToMidY(PointVec point_vec, 
                                  const PairOfPoints dist) {
  int i,j;
  int size = point_vec.length;
  PairOfPoints min_d = dist;

  qsort(point_vec.points, size, sizeof(Point), compareY);

  for (i = 0; i < size; ++i) {
    for (j = i + 1;
         j < size && (point_vec.points[j].y - point_vec.points[i].y) < min_d.distance; j++) {
      if (distance(point_vec.points[i], point_vec.points[j]) < min_d.distance) {
        min_d.distance = distance(point_vec.points[i], point_vec.points[j]);
        min_d.point1 = point_vec.points[i];
        min_d.point2 = point_vec.points[j];
      }
    }
  }

  return min_d;
}


PointVec getBorderPoints(PointVec point_vec, Point border, PairOfPoints d) {
  int n = point_vec.length;
  PointVec res;
  Point mid_set[n];
  int k = 0;
  int i;
  
  for (i = 0; i<n; i++) {
    if (distance_x(point_vec.points[i], border) < d.distance) {
      mid_set[k] = point_vec.points[i];
      k++;
    }
  }

  res.length = k;
  res.points = mid_set;

  return res;
}


PairOfPoints detClosestPoints(PointVec point_vec) {
  int n = point_vec.length;

  // Base cases
  if (n <= 3) {
    return getMinDistance(point_vec);
  }
  
  PairOfPoints dl, dr, d, d_mid;

  int mid = n / 2;
  Point mid_point = point_vec.points[mid];

  PointVec mid1;
  mid1.length = mid;
  mid1.points = point_vec.points;
  
  PointVec mid2;
  mid1.length = point_vec.length - mid;
  mid1.points = point_vec.points + mid;

  // Calculate left and right minimum distances
  dl = detClosestPoints(mid1);
  dr = detClosestPoints(mid2);

  d = dl.distance < dr.distance ? dl : dr;

  // Check points with distance from middle < d
  PointVec mid_set = getBorderPoints(point_vec, mid_point, d);

  d_mid = getMinDistanceToMidY(mid_set, d);

  return d.distance < d_mid.distance ? d : d_mid;
}


PairOfPoints detClosestPointsWrapper(PointVec point_vec) {
  qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);
  return detClosestPoints(point_vec);
}