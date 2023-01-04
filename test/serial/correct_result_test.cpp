#include <gtest/gtest.h>

extern "C" {
#include "../../src/parallel/utils/points.h"
#include "../../src/serial/divide_et_impera.c"
}

// Testing that it finds the right closest point in a small dataset
TEST(Serial, CorrectResult) {
  PointVec input;
  const int size = 8;
  input.length = size;
  input.points = (Point *)malloc(size * sizeof(Point));

  input.points[0] = {1, 2};
  input.points[1] = {-2, -3};
  input.points[2] = {3, 4};
  input.points[3] = {9, 5};
  input.points[4] = {5, 6};
  input.points[5] = {5, 5};
  input.points[6] = {0, 1};
  input.points[7] = {7, 7};

  PairOfPoints result = closest_points(input);
  PairOfPoints expectedResult;
  expectedResult.point1 = {5, 5};
  expectedResult.point2 = {5, 6};
  expectedResult.distance = 1.0;

  EXPECT_EQ(result.distance, expectedResult.distance);

  EXPECT_EQ(result.point1.x, expectedResult.point1.x);
  EXPECT_EQ(result.point1.y, expectedResult.point1.y);

  EXPECT_EQ(result.point2.x, expectedResult.point2.x);
  EXPECT_EQ(result.point2.y, expectedResult.point2.y);
}
