#include <ch.h>
#include <hal.h>

#include "adc.h"
#include "line.h"
#include "throttle.h"

void throttle_init(void) {
  palSetLineMode(LINE_THROTTLE_POWER_SWITCH, PAL_MODE_INPUT_PULLDOWN);
}

bool throttle_power_on(void) {
  return palReadLine(LINE_THROTTLE_POWER_SWITCH) == PAL_HIGH;
}
