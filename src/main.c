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
#include "throttle.h"

static void gpt3_callback(GPTDriver *driver) {
  (void)driver;
  motor_update_callback();
}

// with 26" wheels, and 9 sets of rotor poles, 2500Hz update will allow up to 39mph motion
// go to 3000Hz
static const GPTConfig gpt3cfg = {
  .frequency = 21000,
  .callback = gpt3_callback,
  .cr2 = 0,
  .dier = 0U
};

static void start_motor_update_timer(void) {
  gptStart(&GPTD3, &gpt3cfg);
  gptStartContinuous(&GPTD3, 7);
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

  throttle_init();

  start_motor_update_timer();

  log_println("Initialized");
}

int main(void) {
  init();

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

    if (!throttle_power_on()) {
      log_println("throttle is off");
    } else {
      motor_get_phase_currents(adc_currents);
      log_println("ADC temp %.1fC, Vref %.2fV, throttle %.2f, commutation state %d",
        adc_temp_celsius(),
        adc_vref(),
        adc_throttle_percentage(),
        motor_rotor_tracker_last_commutation_state()
      );
    }

    chThdSleepMilliseconds(500);
  }
}
