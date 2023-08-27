#include "pch.h"

extern "C" {
#include "../../poly_lib/polynomial.c"
}

TEST(TestCaseName, TestName) {
  EXPECT_EQ(test(), 1);
  EXPECT_TRUE(true);
}