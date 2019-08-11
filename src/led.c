#include "ch.h"
#include "hal.h"

#include "led.h"

void led_init(void) {
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOC, 10, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE, 0, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE, 1, PAL_MODE_OUTPUT_PUSHPULL);

  palClearPad(GPIOC, 8);
  palClearPad(GPIOC, 9);
  palClearPad(GPIOC, 10);
  palClearPad(GPIOE, 0);
  palClearPad(GPIOE, 1);
}

void led_turn_on_c8_led(void) {
  palSetPad(GPIOC, 8);
}

void led_turn_off_c8_led(void) {
  palClearPad(GPIOC, 8);
}
