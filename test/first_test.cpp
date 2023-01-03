#include <gtest/gtest.h>

extern "C"{
  //#include "../parallel/main.c"
}

TEST(BasicTest, BasicAssertions) {
  EXPECT_STRNE("hello", "world");
  EXPECT_EQ(7 * 6, 42);
}
