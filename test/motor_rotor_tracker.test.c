#include <unity.h>

#include "motor_rotor_tracker.h"

void test_constrain(void) {
  TEST_ASSERT_EQUAL_FLOAT(0, motor_rotor_tracker_position_revolution_percentage());
  // TODO simulate something
}

