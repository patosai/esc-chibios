#include <ch.h>
#include <hal.h>

#include "led.h"

void led_1_turn_on(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOD, 12);
#else
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 8);
#endif
}

void led_1_toggle(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOD, 12);
#else
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOC, 8);
#endif
}

void led_1_turn_off(void) {
#ifdef DISCOVERY
  palClearPad(GPIOD, 12);
#else
  palClearPad(GPIOC, 8);
#endif
}

void led_2_turn_on(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOD, 13);
#else
  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 9);
#endif
}

void led_2_toggle(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOD, 13);
#else
  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOC, 9);
#endif
}

void led_2_turn_off(void) {
#ifdef DISCOVERY
  palClearPad(GPIOD, 13);
#else
  palClearPad(GPIOC, 9);
#endif
}

void led_3_turn_on(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOD, 14);
#else
  palSetPadMode(GPIOC, 10, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 10);
#endif
}

void led_3_toggle(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOD, 14);
#else
  palSetPadMode(GPIOC, 10, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOC, 10);
#endif
}

void led_3_turn_off(void) {
#ifdef DISCOVERY
  palClearPad(GPIOD, 14);
#else
  palClearPad(GPIOC, 10);
#endif
}

void led_4_turn_on(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOD, 15);
#else
  palSetPadMode(GPIOE, 0, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 0);
#endif
}

void led_4_toggle(void) {
#ifdef DISCOVERY
  palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOD, 15);
#else
  palSetPadMode(GPIOE, 0, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOE, 0);
#endif
}

void led_4_turn_off(void) {
#ifdef DISCOVERY
  palClearPad(GPIOD, 15);
#else
  palClearPad(GPIOE, 0);
#endif
}

void led_5_turn_on(void) {
#ifdef DISCOVERY
#else
  palSetPadMode(GPIOE, 1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 1);
#endif
}

void led_5_toggle(void) {
#ifdef DISCOVERY
#else
  palSetPadMode(GPIOE, 1, PAL_MODE_OUTPUT_PUSHPULL);
  palTogglePad(GPIOE, 1);
#endif
}

void led_5_turn_off(void) {
#ifdef DISCOVERY
#else
  palClearPad(GPIOE, 1);
#endif
}
