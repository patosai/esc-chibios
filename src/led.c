#include <ch.h>
#include <hal.h>

#include "led.h"

void led_1_turn_on(void) {
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 8);
}

void led_1_toggle(void) {
  palSetPadMode(GPIOE, 2, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOE, 2);
}

void led_1_turn_off(void) {
  palClearPad(GPIOE, 2);
}

void led_2_turn_on(void) {
  palSetPadMode(GPIOE, 3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 3);
}

void led_2_toggle(void) {
  palSetPadMode(GPIOE, 3, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOE, 3);
}

void led_2_turn_off(void) {
  palClearPad(GPIOE, 3);
}
