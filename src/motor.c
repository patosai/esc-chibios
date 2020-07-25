#include <ch.h>
#include <hal.h>
#include <math.h>

#include "adc.h"
#include "motor.h"
#include "drv8353rs.h"

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

static float motor_phase_currents_buffer[3];
static uint8_t motor_power_percentage;
static const float sqrt_3_over_2 = sqrt(3)/2.0;

static void setup_pwm_pins(void) {
  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 5, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 6, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, 7, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOC, 4, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOC, 5, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPad(GPIOA, 4);
  palClearPad(GPIOA, 5);
  palSetPad(GPIOA, 6);
  palClearPad(GPIOA, 7);
  palSetPad(GPIOC, 4);
  palClearPad(GPIOC, 5);
}

void motor_init(void) {
  setup_pwm_pins();
  drv8353rs_init();
  motor_power_percentage = 0;
}

void motor_set_power_percentage(uint8_t power_percentage) {
  motor_power_percentage = power_percentage;
}

static float get_rotor_flux_direction_radians(void) {
  // TODO
  return 0;
}

void motor_update_routine(void) {
  adc_retrieve_phase_currents(motor_phase_currents_buffer);

  float i_alpha = motor_phase_currents_buffer[0] - motor_phase_currents_buffer[1]*0.5 - motor_phase_currents_buffer[2]*0.5;
  float i_beta = sqrt_3_over_2*(motor_phase_currents_buffer[1] - motor_phase_currents_buffer[2]);

  float rotor_flux_direction_radians = get_rotor_flux_direction_radians();
  float cos_component = cos(rotor_flux_direction_radians);
  float sin_component = sin(rotor_flux_direction_radians);
  float i_direct = i_alpha*cos_component + i_beta*sin_component;
  float i_quadrature = -1*i_alpha*sin_component + i_beta*cos_component;

  // TODO
  (void)i_direct;
  (void)i_quadrature;
}