#include <ch.h>
#include <hal.h>

#include "led.h"

void led_1_turn_on(void) {
  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 8);
}

void led_1_turn_off(void) {
  palClearPad(GPIOC, 8);
}

void led_2_turn_on(void) {
  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 9);
}

void led_2_turn_off(void) {
  palClearPad(GPIOC, 9);
}

void led_3_turn_on(void) {
  palSetPadMode(GPIOC, 10, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOC, 10);
}

void led_3_turn_off(void) {
  palClearPad(GPIOC, 10);
}

void led_4_turn_on(void) {
  palSetPadMode(GPIOE, 0, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 0);
}

void led_4_turn_off(void) {
  palClearPad(GPIOE, 0);
}

void led_5_turn_on(void) {
  palSetPadMode(GPIOE, 1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOE, 1);
}

void led_5_turn_off(void) {
  palClearPad(GPIOE, 1);
}
