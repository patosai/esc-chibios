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

static float motor_phase_currents_buffer[3];
static float motor_power_percentage = 0;

// produced with [math.sin(2*math.pi*x/100) for x in range(100)] in python
static const float sin_lookup[100] = {0.0, 0.06279051952931337, 0.12533323356430426, 0.1873813145857246, 0.2486898871648548, 0.3090169943749474, 0.3681245526846779, 0.42577929156507266, 0.4817536741017153, 0.5358267949789967, 0.5877852522924731, 0.6374239897486896, 0.6845471059286886, 0.7289686274214116, 0.7705132427757891, 0.8090169943749473, 0.8443279255020151, 0.8763066800438637, 0.9048270524660196, 0.9297764858882513, 0.9510565162951535, 0.9685831611286311, 0.9822872507286886, 0.9921147013144778, 0.9980267284282716, 1.0, 0.9980267284282716, 0.9921147013144779, 0.9822872507286887, 0.9685831611286311, 0.9510565162951536, 0.9297764858882515, 0.9048270524660195, 0.8763066800438635, 0.844327925502015, 0.8090169943749475, 0.7705132427757893, 0.7289686274214116, 0.6845471059286888, 0.6374239897486899, 0.5877852522924732, 0.535826794978997, 0.4817536741017152, 0.4257792915650729, 0.36812455268467814, 0.3090169943749475, 0.24868988716485524, 0.18738131458572455, 0.12533323356430454, 0.06279051952931358, 1.2246467991473532e-16, -0.06279051952931333, -0.1253332335643043, -0.18738131458572477, -0.24868988716485457, -0.3090169943749473, -0.3681245526846779, -0.42577929156507227, -0.4817536741017154, -0.5358267949789964, -0.5877852522924727, -0.6374239897486896, -0.6845471059286884, -0.7289686274214116, -0.7705132427757894, -0.8090169943749473, -0.8443279255020153, -0.8763066800438636, -0.9048270524660198, -0.9297764858882515, -0.9510565162951535, -0.9685831611286312, -0.9822872507286887, -0.9921147013144778, -0.9980267284282716, -1.0, -0.9980267284282716, -0.9921147013144779, -0.9822872507286887, -0.9685831611286311, -0.9510565162951536, -0.9297764858882516, -0.9048270524660199, -0.8763066800438634, -0.8443279255020151, -0.8090169943749476, -0.7705132427757896, -0.7289686274214122, -0.684547105928689, -0.6374239897486896, -0.5877852522924732, -0.5358267949789971, -0.4817536741017161, -0.425779291565073, -0.3681245526846778, -0.3090169943749476, -0.24868988716485535, -0.18738131458572468, -0.12533323356430467, -0.06279051952931326};

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
  pwmDisableChannel(&PWMD1, 2);
}

static void set_phase_b_ticks(pwmcnt_t ticks) {
  pwmEnableChannel(&PWMD1, 1, ticks);
  palSetLine(LINE_PWM_B_COMP);
}

static void disconnect_phase_b(void) {
  palClearLine(LINE_PWM_B_COMP);
  pwmDisableChannel(&PWMD1, 1);
}

static void set_phase_c_ticks(pwmcnt_t ticks) {
  pwmEnableChannel(&PWMD1, 0, ticks);
  palSetLine(LINE_PWM_C_COMP);
}

static void disconnect_phase_c(void) {
  palClearLine(LINE_PWM_C_COMP);
  pwmDisableChannel(&PWMD1, 0);
}

void motor_disconnect(void) {
  disconnect_phase_a();
  disconnect_phase_b();
  disconnect_phase_c();
}

void motor_update_routine(void) {
//  if (motor_power_percentage < 5) {
//    // maybe we should regen brake
//    motor_disconnect();
//    return;
//  }
//

  motor_get_phase_currents(motor_phase_currents_buffer);

  float i_alpha = motor_phase_currents_buffer[0] - motor_phase_currents_buffer[1]*0.5 - motor_phase_currents_buffer[2]*0.5;
  float i_beta = SQRT_3_OVER_2*(motor_phase_currents_buffer[1] - motor_phase_currents_buffer[2]);

  uint16_t percentage = ((uint16_t)(motor_rotor_tracker_position_revolution_percentage())) % 100;
  float cos_component = sin_lookup[(percentage + 25) % 100];
  float sin_component = sin_lookup[percentage];
  float i_direct = i_alpha*cos_component + i_beta*sin_component;
  float i_quadrature = -i_alpha*sin_component + i_beta*cos_component;

  float direct_output = pid_update(&pid_direct, 0, i_direct);
  // quadrature amperage should go from 0 to 20A
  float quadrature_output = pid_update(&pid_quadrature, motor_power_percentage*20/100.0, i_quadrature);

  direct_output = direct_output * BATTERY_VOLTAGE / 100.0;
  quadrature_output = quadrature_output * BATTERY_VOLTAGE / 100.0;

  float v_alpha = direct_output*cos_component - quadrature_output*sin_component;
  float v_beta = direct_output*sin_component + quadrature_output*cos_component;

  // v_phase could go from -1*PID_LOOP_MAX_OUT to PID_LOOP_MAX_OUT
  float v_phase_a = v_alpha;
  float v_phase_b = (v_beta*SQRT_3_OVER_2) - (v_alpha/2);
  float v_phase_c = (-v_beta*SQRT_3_OVER_2) - (v_alpha/2);
  pwmcnt_t pwm_a_ticks = (pwmcnt_t)(scale(v_phase_a, -PID_LOOP_MAX_OUT, PID_LOOP_MAX_OUT, 0, PWM_PERIOD_TICKS));
  pwmcnt_t pwm_b_ticks = (pwmcnt_t)(scale(v_phase_b, -PID_LOOP_MAX_OUT, PID_LOOP_MAX_OUT, 0, PWM_PERIOD_TICKS));
  pwmcnt_t pwm_c_ticks = (pwmcnt_t)(scale(v_phase_c, -PID_LOOP_MAX_OUT, PID_LOOP_MAX_OUT, 0, PWM_PERIOD_TICKS));

  set_phase_a_ticks(pwm_a_ticks);
  set_phase_b_ticks(pwm_b_ticks);
  set_phase_c_ticks(pwm_c_ticks);
}
