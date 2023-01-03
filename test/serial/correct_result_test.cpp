#include <gtest/gtest.h>

extern "C"{
  // #include "../../src/serial/main.c"
}

TEST(Serial, CorrectResult) {
  EXPECT_STRNE("hello", "world");
  EXPECT_EQ(7 * 6, 42);
}
