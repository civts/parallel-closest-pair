#include "points.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Reads the points from the given file and returns them as a PointVec
PointVec loadData(const char *path) {
  unsigned int n;
  FILE *f = NULL;
  int i;
  Point tmp;
  PointVec point_vec;

  // Open file
  f = fopen(path, "r");
  if (f == NULL) {
    printf("Can't open file %s\n", path);
    exit(1);
  }

  // Read number of points
  fscanf(f, "%d", &n);

  if (n > INT_MAX) {
    printf("Too many points: %u. Maximum is %i\n", n, INT_MAX);
    exit(1);
  } else if (n < 2) {
    printf("Too few points. At least two are needed.\n");
    exit(1);
  }

  // Setup vector of points
  point_vec.length = n;
  point_vec.points = malloc(n * sizeof(Point));

  if (point_vec.points == NULL) {
    printf("Out of memory.\n"
           "This machine does not have enough memory for %d points.\n"
           "Exiting X(\n",
           n);
    exit(1);
  }

  // Read points
  for (i = 0; i < n; i++) {
    fscanf(f, "%d %d", &(tmp.x), &(tmp.y));

    point_vec.points[i] = tmp;
  }

  fclose(f);

  return point_vec;
}
