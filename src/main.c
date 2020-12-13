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
#include "line.h"
#include "log.h"

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

static void gpt3_callback(GPTDriver *driver) {
  (void)driver;
  motor_update_routine();
}

static GPTConfig gpt3cfg = {
  .frequency = 10000,
  .callback = gpt3_callback,
};

int main(void) {
  init();

  gptStart(&GPTD3, &gpt3cfg);
  gptStartContinuous(&GPTD3, 2); // run at 5kHz

  float adc_currents[3];

  while (true) {
    if (drv8353rs_has_fault()) {
      led_2_turn_on();
      log_println("ERROR DRV8353RS Fault 1: 0x%x, Fault 2: 0x%x",
        drv8353rs_read_register(FAULT_STATUS_1),
        drv8353rs_read_register(FAULT_STATUS_2)
      );
    } else {
      led_2_turn_off();
    }
    motor_get_phase_currents(adc_currents);
    log_println("DRV8353RS gate drive high: 0x%x",
      drv8353rs_read_register(GATE_DRIVE_HIGH_CONTROL)
    );
    log_println("ADC temp %.1fC, Vref %.2fV, phase A %.2fA, phase B %.2fA, phase C %.2fA",
      adc_temp_celsius(),
      adc_vref(),
      adc_currents[0],
      adc_currents[1],
      adc_currents[2]
    );
    chThdSleepMilliseconds(1000);
  }
}
