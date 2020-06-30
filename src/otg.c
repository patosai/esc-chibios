#include <ch.h>
#include <hal.h>

#include "otg.h"

void otg_setup(void) {
  // TODO
  palSetPadMode(GPIOA, 11, PAL_MODE_ALTERNATE(10));
  palSetPadMode(GPIOA, 12, PAL_MODE_ALTERNATE(10));
}
