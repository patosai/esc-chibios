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

#include <stdbool.h>

#include "adc.h"
#include "drv8353rs.h"
#include "motor.h"
#include "motor_rotor_tracker.h"
#include "led.h"
#include "line.h"
#include "log.h"

static THD_WORKING_AREA(motorUpdateThreadWorkingArea, 128);

static void init_throttle_power_switch(void) {
  palSetLineMode(LINE_THROTTLE_POWER_SWITCH, PAL_MODE_INPUT_PULLDOWN);
}

static bool throttle_power_on(void) {
  return palReadLine(LINE_THROTTLE_POWER_SWITCH) == PAL_HIGH;
}

static THD_FUNCTION(motorUpdateThread, arg) {
  (void)arg;
  while (true) {
    if (throttle_power_on()) {
      motor_set_power_percentage(adc_throttle_percentage());
      motor_update_routine();
    } else {
      motor_disconnect();
    }
    chThdSleepMilliseconds(1);
  }
}

static void start_motor_update_thread(void) {
  chThdCreateStatic(
    motorUpdateThreadWorkingArea,
    sizeof(motorUpdateThreadWorkingArea),
    HIGHPRIO,
    motorUpdateThread,
    NULL
  );
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

  init_throttle_power_switch();

  log_println("Initialized");
}

int main(void) {
  init();

  start_motor_update_thread();

  float adc_currents[3];

  while (true) {
    if (!throttle_power_on()) {
      log_println("throttle is off");
    } else {
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
      log_println("ADC temp %.1fC, Vref %.2fV, throttle %.2f, commutation state %d",
        adc_temp_celsius(),
        adc_vref(),
        adc_throttle_percentage(),
        motor_rotor_tracker_last_commutation_state()
      );
    }

    chThdSleepMilliseconds(1000);
  }
}
