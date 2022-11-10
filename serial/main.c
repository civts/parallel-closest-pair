#include "divide_et_impera.c"
#include "naive.c"
#include "points_loader.c"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  debugPrint("Started");
  int i;
  clock_t begin, end;
  double time_spent;
  PairOfPoints closestPair;

  debugPrint("Reading the points from file");
  const PointVec point_vec = loadData("../data/test_dataset.txt");

  debugPrint("Sorting the points based on x coordinate");
  qsort(point_vec.points, point_vec.length, sizeof(Point), compareX);

  // debugPrint("Printing the points");
  // for (i = 0; i < point_vec.length; i++) {
  //   printf("%d %d\n", point_vec.points[i].x, point_vec.points[i].y);
  // }

  debugPrint("Getting the closest points with naive algorithm");
  begin = clock();
  closestPair = naiveClosestPoints(point_vec);
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("Found the closest pair in %.2f milliseconds.\n"
         "The closest pair of points is:\n"
         "(%d, %d) and (%d, %d)\n"
         "Their distance is:\n"
         "%f\n",
         time_spent, closestPair.point1.x, closestPair.point1.y,
         closestPair.point2.x, closestPair.point2.y, closestPair.distance);

  debugPrint("Getting the closest points with divide et impera algorithm");
  begin = clock();
  closestPair = detClosestPoints(point_vec.points, point_vec.length);
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("Found the closest pair in %.2f milliseconds.\n"
         "The closest pair of points is:\n"
         "(%d, %d) and (%d, %d)\n"
         "Their distance is:\n"
         "%f\n",
         time_spent, closestPair.point1.x, closestPair.point1.y,
         closestPair.point2.x, closestPair.point2.y, closestPair.distance);

  debugPrint("Deallocating the points vector");
  free(point_vec.points);

  debugPrint("All done");
  return 0;
}

// Dataset: https://cs.joensuu.fi/sipu/datasets/