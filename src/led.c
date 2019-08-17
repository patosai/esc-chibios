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

  // for discovery purposes
  palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOD, 12);
  palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOD, 13);
  palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOD, 14);
  palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOD, 15);
}

void led_turn_on_led1(void) { palSetPad(GPIOC, 8); }
void led_turn_off_led1(void) { palClearPad(GPIOC, 8); }

void led_turn_on_led2(void) { palSetPad(GPIOC, 9); }
void led_turn_off_led2(void) { palClearPad(GPIOC, 9); }

void led_turn_on_led3(void) { palSetPad(GPIOC, 10); }
void led_turn_off_led3(void) { palClearPad(GPIOC, 10); }

void led_turn_on_led4(void) { palSetPad(GPIOE, 0); }
void led_turn_off_led4(void) { palClearPad(GPIOE, 0); }

void led_turn_on_led5(void) { palSetPad(GPIOE, 1); }
void led_turn_off_led5(void) { palClearPad(GPIOE, 1); }


void led_turn_on_discovery_led_green(void) { palSetPad(GPIOD, 12); }
void led_turn_off_discovery_led_green(void) { palClearPad(GPIOD, 12); }

void led_turn_on_discovery_led_orange(void) { palSetPad(GPIOD, 13); }
void led_turn_off_discovery_led_orange(void) { palClearPad(GPIOD, 13); }

void led_turn_on_discovery_led_red(void) { palSetPad(GPIOD, 14); }
void led_turn_off_discovery_led_red(void) { palClearPad(GPIOD, 14); }

void led_turn_on_discovery_led_blue(void) { palSetPad(GPIOD, 15); }
void led_turn_off_discovery_led_blue(void) { palClearPad(GPIOD, 15); }

