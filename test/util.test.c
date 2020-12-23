#include <unity.h>

#include "util.h"

void test_constrain(void) {
  TEST_ASSERT_EQUAL_FLOAT(7.2, constrain(7.2, 7, 8));
  TEST_ASSERT_EQUAL_FLOAT(7.3, constrain(7.2, 7.3, 8));
  TEST_ASSERT_EQUAL_FLOAT(7, constrain(7.2, 6.5, 7.0));

  TEST_ASSERT_EQUAL_FLOAT(-1, constrain(-1, -2, 3));
  TEST_ASSERT_EQUAL_FLOAT(-1.5, constrain(-1, -2, -1.5));
  TEST_ASSERT_EQUAL_FLOAT(-0.75, constrain(-1, -0.75, -0.25));
}

void test_scale(void) {
  TEST_ASSERT_EQUAL_FLOAT(5.5, scale(1, 0, 2, 5, 6));
  TEST_ASSERT_FLOAT_WITHIN(1E-6, 0, scale(-2.3, -2.42, -1.22, -1, 9));
}