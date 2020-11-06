#include <ch.h>
#include <hal.h>

#include "led.h"

void led_1_turn_on(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOE, 2, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 2);
#else
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 8);
#endif
}

void led_1_toggle(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOE, 2, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOE, 2);
#else
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOC, 8);
#endif
}

void led_1_turn_off(void) {
#ifdef DISCOVERY
  palClearPad(GPIOE, 2);
#else
  palClearPad(GPIOC, 8);
#endif
}

void led_2_turn_on(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOE, 3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 3);
#else
  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 9);
#endif
}

void led_2_toggle(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOE, 3, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOE, 3);
#else
  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOC, 9);
#endif
}

void led_2_turn_off(void) {
#ifdef DISCOVERY
  palClearPad(GPIOE, 3);
#else
  palClearPad(GPIOC, 9);
#endif
}
