#include "../utils/utils.h"

/// Finds the closest pair of points in the provided point vector
/// in O(n^2). More precisely, it performs ((n^2)/2)-n comparisons
PairOfPoints naiveClosestPoints(const PointVec vec) {
  const Point *points = vec.points;
  const int n = vec.length;
  PairOfPoints r;
  double min_d = distance(points[0], points[1]);
  min_d *= 2; // ensuring it is larger than the minimum distance
  min_d += 1; // ensuring it is larger than the minimum distance
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (i != j && i < j) {
        double d = distance(points[i], points[j]);
        if (d < min_d) {
          min_d = d;
          r.point1 = points[i];
          r.point2 = points[j];
        }
      }
    }
  }
  r.distance = min_d;
  return r;
}
