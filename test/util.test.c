#include <unity.h>

#include "util.h"

void test_constrain(void) {
  TEST_ASSERT_EQUAL_FLOAT(7.2, constrain(7.2, 7, 8));
  TEST_ASSERT_EQUAL_FLOAT(7.3, constrain(7.2, 7.3, 8));
  TEST_ASSERT_EQUAL_FLOAT(7, constrain(7.2, 6.5, 7.0));
}