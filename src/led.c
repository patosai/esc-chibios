#include <ch.h>
#include <hal.h>

#include "led.h"
#include "line.h"

void led_1_turn_on(void) {
  palSetLineMode(LINE_LED_1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLine(LINE_LED_1);
}

void led_1_toggle(void) {
  palSetLineMode(LINE_LED_1, PAL_MODE_OUTPUT_PUSHPULL);
  palToggleLine(LINE_LED_1);
}

void led_1_turn_off(void) {
  palClearLine(LINE_LED_1);
}

void led_2_turn_on(void) {
  palSetLineMode(LINE_LED_2, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLine(LINE_LED_2);
}

void led_2_toggle(void) {
  palSetLineMode(LINE_LED_2, PAL_MODE_OUTPUT_PUSHPULL);
  palToggleLine(LINE_LED_2);
}

void led_2_turn_off(void) {
  palClearLine(LINE_LED_2);
}
