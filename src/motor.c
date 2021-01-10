#include <ch.h>
#include <hal.h>
#include <math.h>

#include "adc.h"
#include "constants.h"
#include "drv8353rs.h"
#include "line.h"
#include "log.h"
#include "motor.h"
#include "motor_rotor_tracker.h"
#include "pid.h"
#include "util.h"

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

// IMPORTANT: clock frequency MUST divide APB2 evenly!
// clock frequency = PWM_FREQUENCY_HZ * PWM_PERIOD_TICKS
// APB2 was 42MHz last time I checked
#define PWM_FREQUENCY_HZ 50000
#define PWM_PERIOD_TICKS 840
//#define SQRT_3_OVER_2 sqrt(3)/2.0
#define SQRT_3_OVER_2 0.866

#define PID_LOOP_P 5
#define PID_LOOP_I 0.05
#define PID_LOOP_D 0
#define PID_LOOP_MIN_OUT 0
#define PID_LOOP_MAX_OUT 100

static pid_state_t pid_direct;
static pid_state_t pid_quadrature;

static float motor_power_percentage = 0;

// offsets were found experimentally
//static float motor_current_offsets[ADC_MOTOR_PHASES_SAMPLED] = {-0.5, 0.98};

static PWMConfig pwm_config = {
  .frequency = PWM_PERIOD_TICKS*PWM_FREQUENCY_HZ,
  .period = PWM_PERIOD_TICKS,
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

void motor_init(void) {
  setup_pwm();

  motor_rotor_tracker_setup();

  drv8353rs_init();

  pid_direct = pid_create(PID_LOOP_P, PID_LOOP_I, PID_LOOP_D, PID_LOOP_MIN_OUT, PID_LOOP_MAX_OUT);
  pid_quadrature = pid_create(PID_LOOP_P, PID_LOOP_I, PID_LOOP_D, PID_LOOP_MIN_OUT, PID_LOOP_MAX_OUT);

  pid_reset(&pid_direct);
  pid_reset(&pid_quadrature);
}

void motor_set_power_percentage(float percentage) {
  motor_power_percentage = constrain(percentage, 0, 100);
}

void motor_get_phase_currents(float* buf) {
  // buf must be length 3: [phase A, phase B, phase C]
  adc_get_phase_voltages(buf);

  // formula for converting ADC voltage to current
  // DRV takes -0.15 to 0.15V, amplifies it (changeable via setting), and outputs 0 to 3.3V
  const float current_factor = DRV_CURRENT_SENSE_AMPLIFICATION * PHASE_RESISTANCE_OHMS;
  buf[0] = (DRV_REFERENCE_VOLTAGE - buf[0])/current_factor;
  buf[1] = (DRV_REFERENCE_VOLTAGE - buf[1])/current_factor;

  buf[2] = buf[1]; // + motor_current_offsets[1];
  buf[1] = buf[0]; // + motor_current_offsets[0];
  buf[0] = -(buf[1] + buf[2]);
}

static void set_phase_a_ticks(pwmcnt_t ticks) {
  pwmEnableChannel(&PWMD1, 2, ticks);
  palSetLine(LINE_PWM_A_COMP);
}

static void disconnect_phase_a(void) {
  palClearLine(LINE_PWM_A_COMP);
//  pwmDisableChannel(&PWMD1, 2);
}

static void set_phase_b_ticks(pwmcnt_t ticks) {
  pwmEnableChannel(&PWMD1, 1, ticks);
  palSetLine(LINE_PWM_B_COMP);
}

static void disconnect_phase_b(void) {
  palClearLine(LINE_PWM_B_COMP);
//  pwmDisableChannel(&PWMD1, 1);
}

static void set_phase_c_ticks(pwmcnt_t ticks) {
  pwmEnableChannel(&PWMD1, 0, ticks);
  palSetLine(LINE_PWM_C_COMP);
}

static void disconnect_phase_c(void) {
  palClearLine(LINE_PWM_C_COMP);
//  pwmDisableChannel(&PWMD1, 0);
}

void motor_disconnect(void) {
  disconnect_phase_a();
  disconnect_phase_b();
  disconnect_phase_c();
}

void motor_update_routine(void) {
  motor_set_power_percentage(adc_throttle_percentage());

  if (motor_power_percentage < 5) {
    // maybe we should regen brake
    motor_disconnect();
  } else {
    pwmcnt_t ticks = constrain(motor_power_percentage / 100.0 * PWM_PERIOD_TICKS, 0, PWM_PERIOD_TICKS);

    // TODO why does the system crash when the following is removed..?
    if (ticks > 20) {
      // prevent test blowup
      ticks = 20;
    }

    pwmcnt_t complementary_ticks = PWM_PERIOD_TICKS - ticks;

    uint8_t commutation_state = motor_rotor_tracker_last_commutation_state();
    switch (commutation_state) {
      case 0:
        set_phase_a_ticks(ticks);
        set_phase_b_ticks(complementary_ticks);
        disconnect_phase_c();
        break;
      case 1:
        set_phase_a_ticks(ticks);
        disconnect_phase_b();
        set_phase_c_ticks(complementary_ticks);
        break;
      case 2:
        disconnect_phase_a();
        set_phase_b_ticks(ticks);
        set_phase_c_ticks(complementary_ticks);
        break;
      case 3:
        set_phase_a_ticks(complementary_ticks);
        set_phase_b_ticks(ticks);
        disconnect_phase_c();
        break;
      case 4:
        set_phase_a_ticks(complementary_ticks);
        disconnect_phase_b();
        set_phase_c_ticks(ticks);
        break;
      case 5:
        disconnect_phase_a();
        set_phase_b_ticks(complementary_ticks);
        set_phase_c_ticks(ticks);
        break;
      default:
        motor_disconnect();
        break;
    }
  }
}

// FOC routine - use at a future point in time
//void motor_update_routine(void) {
//  if (motor_power_percentage < 5) {
//    // maybe we should regen brake
//    motor_disconnect();
//    return;
//  }
//  motor_get_phase_currents(motor_phase_currents_buffer);
//
//  float i_alpha = motor_phase_currents_buffer[0] - motor_phase_currents_buffer[1]*0.5 - motor_phase_currents_buffer[2]*0.5;
//  float i_beta = SQRT_3_OVER_2*(motor_phase_currents_buffer[1] - motor_phase_currents_buffer[2]);
//
//  uint16_t percentage = ((uint16_t)(motor_rotor_tracker_position_revolution_percentage())) % 100;
//  float cos_component = sin_lookup[(percentage + 25) % 100];
//  float sin_component = sin_lookup[percentage];
//  float i_direct = i_alpha*cos_component + i_beta*sin_component;
//  float i_quadrature = -i_alpha*sin_component + i_beta*cos_component;
//  log_println("cos %.2f, sin %.2f", cos_component, sin_component);
//
//  float direct_output = pid_update(&pid_direct, 0, i_direct);
//  // quadrature amperage should go from 0 to 20A
//  float quadrature_output = pid_update(&pid_quadrature, motor_power_percentage*20/100.0, i_quadrature);
//
//  direct_output = direct_output * BATTERY_VOLTAGE / 100.0;
//  quadrature_output = quadrature_output * BATTERY_VOLTAGE / 100.0;
//
//  float v_alpha = direct_output*cos_component - quadrature_output*sin_component;
//  float v_beta = direct_output*sin_component + quadrature_output*cos_component;
//
//  // v_phase could go from -1*PID_LOOP_MAX_OUT to PID_LOOP_MAX_OUT
//  float v_phase_a = v_alpha;
//  float v_phase_b = (v_beta*SQRT_3_OVER_2) - (v_alpha/2);
//  float v_phase_c = (-v_beta*SQRT_3_OVER_2) - (v_alpha/2);
//  pwmcnt_t pwm_a_ticks = (pwmcnt_t)(scale(v_phase_a, -PID_LOOP_MAX_OUT, PID_LOOP_MAX_OUT, 0, PWM_PERIOD_TICKS));
//  pwmcnt_t pwm_b_ticks = (pwmcnt_t)(scale(v_phase_b, -PID_LOOP_MAX_OUT, PID_LOOP_MAX_OUT, 0, PWM_PERIOD_TICKS));
//  pwmcnt_t pwm_c_ticks = (pwmcnt_t)(scale(v_phase_c, -PID_LOOP_MAX_OUT, PID_LOOP_MAX_OUT, 0, PWM_PERIOD_TICKS));
//
//  set_phase_a_ticks(pwm_a_ticks);
//  set_phase_b_ticks(pwm_b_ticks);
//  set_phase_c_ticks(pwm_c_ticks);
//}
