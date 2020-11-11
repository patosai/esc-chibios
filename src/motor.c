#include <ch.h>
#include <hal.h>
#include <math.h>

#include "adc.h"
#include "constants.h"
#include "drv8353rs.h"
#include "line.h"
#include "log.h"
#include "motor.h"
#include "pid.h"

// CONVENTION: Phase A is 0 rad, B is 2pi/3 rad, C is 4pi/3 rad

/*Field oriented control overview
===================================

  \  B
   \
    \______ A
    /
   /
  /  C

  Given a three phase motor, the stator currents/flux in A, B, and C can be transformed into two currents/fluxes in a 2D frame.

  | β
  |
  |
  |_______ α

  I(α) = I(A) - I(B)*sin(pi/6) - I(C)*sin(pi/6) = I(A) - I(B)/2 - I(C)/2
  I(β) = I(B)*cos(pi/6) - I(C)*cos(pi/6) = (I(B) - I(C)) * sqrt(3)/2

  This is the Clarke transform.

  ----------------

  Because the stator flux acts on a rotating rotor, the 2D frame can be transformed into another 2D frame, but one that
  is always aligned with the rotor. This way the fluxes can be compared more directly.

  ＼  q         ／ d
    ＼        ／
      ＼    ／
        ＼／_φ _  _  _

  Assume for now that the position of the rotor can be calculated. Let d be the direction of the rotor's north pole.
  Let φ be the angle between the αβ and dq frame. Then

  I(d) = I(a)*cos(φ) + I(b)*sin(φ)
  I(q) = -I(a)*sin(φ) + I(b)*cos(φ)

  This is the Park transform.

  I(d) is known as the direct current and I(q) is the quadrature current. I(d) points directly where the rotor flux is pointing,
  and in a synchronous motor like a BLDC, does no work. I(q) is the torque generating current.

  PID loops are used to try to control I(d) and I(q) to levels where we want.

  The results of the PID loops are then fed into the inverse Park transform, then into the inverse Clarke transform to

 */

#define PWM_FREQUENCY_HZ 84000
#define PWM_PERIOD_TICKS_MAX 100
#define SQRT_3_OVER_2 sqrt(3)/2.0

static pid_state_t pid_direct;
static pid_state_t pid_quadrature;

//static float motor_phase_currents_buffer[3];
static uint16_t motor_pwm_period_ticks;

static PWMConfig pwm_config = {
  .frequency = PWM_PERIOD_TICKS_MAX*PWM_FREQUENCY_HZ,
  .period = PWM_PERIOD_TICKS_MAX,
  .callback = NULL,
  .channels = {
    {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},
    {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},
    {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},
    {.mode = PWM_OUTPUT_DISABLED, .callback = NULL}
  }
};

static void setup_pwm(void) {
  pwmStart(&PWMD1, &pwm_config);
  for (uint8_t channel = 0; channel < 3; ++channel) {
    pwmEnableChannel(&PWMD1, channel, 0);
  }

  palSetLineMode(LINE_PWM_A, PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_PWM_A_COMP, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_PWM_B, PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_PWM_B_COMP, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_PWM_C, PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_PWM_C_COMP, PAL_MODE_OUTPUT_PUSHPULL);

  palClearLine(LINE_PWM_A_COMP);
  palClearLine(LINE_PWM_B_COMP);
  palClearLine(LINE_PWM_C_COMP);
}

static void setup_hall_sensors(void) {
  palSetLineMode(LINE_HALL_SENSOR_A, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(LINE_HALL_SENSOR_B, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(LINE_HALL_SENSOR_C, PAL_MODE_INPUT_PULLUP);
}

void motor_init(void) {
  setup_pwm();
  setup_hall_sensors();
  drv8353rs_init();
  motor_pwm_period_ticks = 0;

  pid_direct = pid_create(2, 0, 0);
  pid_quadrature = pid_create(2, 0, 0);

  pid_reset(&pid_direct);
  pid_reset(&pid_quadrature);
}

void motor_set_power_percentage(float power_percentage) {
  power_percentage = power_percentage > 100 ? 100 : power_percentage;
  uint16_t ticks = (uint16_t)(power_percentage * PWM_PERIOD_TICKS_MAX);
  ticks = ticks > PWM_PERIOD_TICKS_MAX ? PWM_PERIOD_TICKS_MAX : ticks;
  motor_pwm_period_ticks = ticks;
}

static uint8_t last_commutation_result = 0;
static uint8_t get_rotor_commutation_state(void) {
  bool a_high = palReadLine(LINE_HALL_SENSOR_A) == PAL_HIGH;
  bool b_high = palReadLine(LINE_HALL_SENSOR_B) == PAL_HIGH;
  bool c_high = palReadLine(LINE_HALL_SENSOR_C) == PAL_HIGH;
  uint8_t result = a_high << 2 & b_high << 1 & c_high;
  switch (result) {
  case 0b100:
    last_commutation_result = 0;
    break;
  case 0b110:
    last_commutation_result = 1;
    break;
  case 0b010:
    last_commutation_result = 2;
    break;
  case 0b011:
    last_commutation_result = 3;
    break;
  case 0b001:
    last_commutation_result = 4;
    break;
  case 0b101:
    last_commutation_result = 5;
    break;
  default:
    log_println_in_interrupt("Unknown commutation result 0x%x", result);
    break;
  }
  return last_commutation_result;
}

//
// TODO reenable FOC on V1.1 esc
//void motor_update_routine(void) {
//  adc_retrieve_phase_currents(motor_phase_currents_buffer);
//
//  float i_alpha = motor_phase_currents_buffer[0] - motor_phase_currents_buffer[1]*0.5 - motor_phase_currents_buffer[2]*0.5;
//  float i_beta = SQRT_3_OVER_2*(motor_phase_currents_buffer[1] - motor_phase_currents_buffer[2]);
//
//  float rotor_flux_direction_radians = get_rotor_flux_direction_radians();
//  float cos_component = cos(rotor_flux_direction_radians);
//  float sin_component = sin(rotor_flux_direction_radians);
//  float i_direct = i_alpha*cos_component + i_beta*sin_component;
//  float i_quadrature = -i_alpha*sin_component + i_beta*cos_component;
//
//  // TODO fix D/Q expected values
//  float direct_output = pid_update(&pid_direct, 0, i_direct);
//  // quadrature amperage should go from 0 to 15A
//  float quadrature_output = pid_update(&pid_quadrature, 1, i_quadrature);
//
//  float v_alpha = direct_output*cos_component - quadrature_output*sin_component;
//  float v_beta = direct_output*sin_component + quadrature_output*cos_component;
//
//  float v_phase_1 = v_alpha;
//  float v_phase_2 = v_beta*SQRT_3_OVER_2 - v_alpha/2;
//  float v_phase_3 = -v_beta*SQRT_3_OVER_2 - v_alpha/2;
//
//  pwmEnableChannel(&PWMD1, 0, (pwmcnt_t)(v_phase_1/BATTERY_VOLTAGE * PWM_PERIOD_TICKS_MAX));
//  pwmEnableChannel(&PWMD1, 1, (pwmcnt_t)(v_phase_2/BATTERY_VOLTAGE * PWM_PERIOD_TICKS_MAX));
//  pwmEnableChannel(&PWMD1, 2, (pwmcnt_t)(v_phase_3/BATTERY_VOLTAGE * PWM_PERIOD_TICKS_MAX));
//}

//static void set_phase_a_ticks(uint16_t ticks) {
//  pwmEnableChannel(&PWMD1, 2, ticks);
//  palSetPad(GPIOE, 8);
//}
//
//static void disconnect_phase_a(void) {
//  palClearPad(GPIOE, 8);
//  pwmDisableChannel(&PWMD1, 2);
//}
//
//static void set_phase_b_ticks(uint16_t ticks) {
//  pwmEnableChannel(&PWMD1, 1, ticks);
//  palSetPad(GPIOE, 10);
//}
//
//static void disconnect_phase_b(void) {
//  palClearPad(GPIOE, 10);
//  pwmDisableChannel(&PWMD1, 1);
//}
//
//static void set_phase_c_ticks(uint16_t ticks) {
//  pwmEnableChannel(&PWMD1, 0, ticks);
//  palSetPad(GPIOE, 12);
//}
//
//static void disconnect_phase_c(void) {
//  palClearPad(GPIOE, 12);
//  pwmDisableChannel(&PWMD1, 0);
//}

void motor_update_routine(void) {
  // TODO
  get_rotor_commutation_state();
}