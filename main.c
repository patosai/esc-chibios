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

#include "chprintf.h"

#include "adc.h"
#include "led.h"
#include "spi.h"
#include "usart.h"

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

static THD_WORKING_AREA(waThreadSerialReporting, 128);
static THD_FUNCTION(ThreadSerialReporting, arg) {
  (void)arg;
  chRegSetThreadName("serial_reporting");
  while (true) {
    serial2_send("VREF: %.3f\r\n", adc_vref());
    serial2_send("phase a voltage: %.3f\r\n", adc_phase_a_voltage());
    chThdSleepMilliseconds(1000);
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

  adc_init();
  led_init();
  spi_init();
  serial_init();
}

static void create_threads(void) {
  chThdCreateStatic(waThreadLedBlinker, sizeof(waThreadLedBlinker), NORMALPRIO, ThreadLedBlinker, NULL);
  chThdCreateStatic(waThreadSerialReporting, sizeof(waThreadSerialReporting), NORMALPRIO, ThreadSerialReporting, NULL);
}

/*
 * Application entry point.
 */
int main(void) {
  init();

  create_threads();

  adc_start_current_measurement_conversion();

  while (true) {
    led_turn_on_discovery_led_orange();
    chThdSleepMilliseconds(1000);
    led_turn_off_discovery_led_orange();
    chThdSleepMilliseconds(1000);
  }
}
