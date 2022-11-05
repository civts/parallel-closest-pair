#ifndef __UTILS_H__
#define __UTILS_H__

#include <math.h>
#include <stdio.h>

const short DEBUGGING = 1;

typedef struct {
  int x, y;
} Point;

typedef struct {
  int length;
  Point *points;
} PointVec;

typedef struct {
  double distance; // TODO: test if it is better to include this field or to
                   // calculate it when needed
  Point point1;
  Point point2;
} PairOfPoints;

// Compare two points based on their x coordinate
int compareX(const void *e1, const void *e2) {
  Point *p1 = (Point *)e1;
  Point *p2 = (Point *)e2;
  return (p1->x - p2->x);
}

// Returns the distance between the two given points
double distance(const Point p1, const Point p2) {
  return sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
}

// Utility function to print to standard output only if we are debugging
void debugPrint(const char *message) {
  if (DEBUGGING) {
    printf("❱❱ %s\n", message);
  }
}

#endif
