/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"

#include "adc.h"
#include "led.h"
#include "spi.h"
#include "usart.h"

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThreadLedBlinker, 128);
static THD_FUNCTION(ThreadLedBlinker, arg) {
  (void)arg;
  chRegSetThreadName("led_blinker");
  while (true) {
    led_turn_on_discovery_led_green();
    chThdSleepMilliseconds(500);

    led_turn_off_discovery_led_green();
    chThdSleepMilliseconds(500);
  }
}

static void init(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  led_init();
  spi2_init();
  usart1_init();
}

static void create_threads(void) {
  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThreadLedBlinker, sizeof(waThreadLedBlinker), NORMALPRIO, ThreadLedBlinker, NULL);
}

/*
 * Application entry point.
 */
int main(void) {
  init();

  adc_start_current_measurement_conversion();

  create_threads();

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    led_turn_on_discovery_led_orange();
    chThdSleepMilliseconds(1000);
    led_turn_off_discovery_led_orange();
    chThdSleepMilliseconds(1000);
  }
}
