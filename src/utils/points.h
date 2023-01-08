#ifndef __POINTS_H__
#define __POINTS_H__ 1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const short DEBUGGING = 1;

typedef struct {
  int x, y;
} Point;

typedef struct {
  int length;
  Point *points;
} PointVec;

typedef struct {
  double distance;
  Point point1;
  Point point2;
} PairOfPoints;

// Compare two points based on their x coordinate
int compareX(const void *e1, const void *e2) {
  Point *p1 = (Point *)e1;
  Point *p2 = (Point *)e2;
  return (p1->x - p2->x);
}

// Compare two points based on their y coordinate
int compareY(const void *e1, const void *e2) {
  Point *p1 = (Point *)e1;
  Point *p2 = (Point *)e2;
  return (p1->y - p2->y);
}

// Returns the distance between the two given points
double distance(const Point p1, const Point p2) {
  return sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
}

// Returns the distance between the two given points, x coord
double distance_x(const Point p1, const Point p2) { return abs(p1.x - p2.x); }

// Creates a pair from two points
PairOfPoints points_to_pair(Point point1, Point point2) {
  PairOfPoints result;
  result.point1 = point1;
  result.point2 = point2;
  result.distance = distance(result.point1, result.point2);
  return result;
}

// Utility function to print to standard output only if we are debugging
void debugPrint(const char *message) {
  if (DEBUGGING) {
    printf("❱❱ %s\n", message);
  }
}

#endif
