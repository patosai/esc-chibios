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

#include "drv8353rs.h"
#include "led.h"
#include "serial.h"

static THD_WORKING_AREA(waThreadLedBlinker, 128);
static THD_FUNCTION(ThreadLedBlinker, arg) {
  (void)arg;
  chRegSetThreadName("led_blinker");
  while (true) {
    led_2_turn_on();
    chThdSleepMilliseconds(500);

    led_2_turn_off();
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

  serial1_init();
  serial1_send("Initialized serial");

  drv8353rs_init();
}

static void create_threads(void) {
  chThdCreateStatic(waThreadLedBlinker, sizeof(waThreadLedBlinker), NORMALPRIO, ThreadLedBlinker, NULL);
}

int main(void) {
  init();
  create_threads();

  while (true) {
    if (drv8353rs_has_fault()) {
      led_3_turn_on();
    } else {
      led_3_turn_off();
    }
    chThdSleepMilliseconds(1000);

    serial1_send("0x%x", drv8353rs_read_register(FAULT_STATUS_1));
    chThdSleepMilliseconds(10);
    serial1_send("0x%x", drv8353rs_read_register(FAULT_STATUS_2));
  }
}
