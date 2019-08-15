#include "ch.h"
#include "hal.h"

#include "usart.h"

void usart1_init(void) {
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
  sdStart(&SD1, NULL);
}
