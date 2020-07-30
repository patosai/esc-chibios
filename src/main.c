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

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "adc.h"
#include "drv8353rs.h"
#include "motor.h"
#include "led.h"
#include "log.h"

static THD_WORKING_AREA(waThreadLedBlinker, 128);
static THD_FUNCTION(ThreadLedBlinker, arg) {
  (void)arg;
  chRegSetThreadName("led_blinker");
  while (true) {
    led_2_toggle();
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

  log_init();
  motor_init();

  log_println("Initialized");
}

static void create_threads(void) {
  chThdCreateStatic(waThreadLedBlinker, sizeof(waThreadLedBlinker), LOWPRIO, ThreadLedBlinker, NULL);
}

int main(void) {
  init();
  create_threads();

  adc_start_continuous_conversion();

  while (true) {
    if (drv8353rs_has_fault()) {
      led_3_turn_on();
    } else {
      led_3_turn_off();
    }
    chThdSleepMilliseconds(1000);

    log_println("ADC temp %.1fC", adc_temp());
    log_println("ADC Vref %.2fV", adc_vref());
  }
}
