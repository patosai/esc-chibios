#include <ch.h>
#include <hal.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "line.h"
#include "motor_rotor_tracker.h"

#define TRACKER_UPDATE_FREQUENCY_HZ 10000
static const float alpha = 0.7;
static const float beta = 0.01;
static const float dt = 1/TRACKER_UPDATE_FREQUENCY_HZ;

static float alpha_integrator_value = 0;
static float beta_integrator_value = 0;

static float last_position;
static float last_velocity;
static float last_acceleration;

typedef uint8_t commutation_state_t;
static commutation_state_t last_commutation_state = 0;

static commutation_state_t get_commutation_state(void) {
  bool a_high = palReadLine(LINE_HALL_SENSOR_A);
  bool b_high = palReadLine(LINE_HALL_SENSOR_B);
  bool c_high = palReadLine(LINE_HALL_SENSOR_C);
  uint8_t compare_val = a_high << 2 | b_high << 1 | c_high << 0;
  switch (compare_val) {
    case 0b101:
      return 0;
    case 0b100:
      return 1;
    case 0b110:
      return 2;
    case 0b010:
      return 3;
    case 0b011:
      return 4;
    case 0b001:
      return 5;
    default:
      return last_commutation_state;
  }
}

static void hall_sensor_change_callback(void* input) {
  (void)input;
  last_commutation_state = get_commutation_state();
}

static void setup_hall_sensors(void) {
  palSetLineMode(LINE_HALL_SENSOR_A, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(LINE_HALL_SENSOR_B, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(LINE_HALL_SENSOR_C, PAL_MODE_INPUT_PULLUP);

  palSetLineCallback(LINE_HALL_SENSOR_A, (palcallback_t)hall_sensor_change_callback, NULL);
  palSetLineCallback(LINE_HALL_SENSOR_B, (palcallback_t)hall_sensor_change_callback, NULL);
  palSetLineCallback(LINE_HALL_SENSOR_C, (palcallback_t)hall_sensor_change_callback, NULL);

  palEnableLineEvent(LINE_HALL_SENSOR_A, PAL_EVENT_MODE_BOTH_EDGES);
}

static void update_alpha_beta_filter(void) {
  float error = last_commutation_state - alpha_integrator_value;
  float beta_update = beta * error;
  beta_integrator_value += dt * beta_update;

  float alpha_update = alpha * error + beta_integrator_value;
  alpha_integrator_value += dt * alpha_update;

  last_position = alpha_integrator_value;
  last_velocity = alpha_update;
  last_acceleration = beta_update;
}

static void gpt4_callback(GPTDriver *driver) {
  (void)driver;
  update_alpha_beta_filter();
}

static GPTConfig gpt4cfg = {
  .frequency = TRACKER_UPDATE_FREQUENCY_HZ,
  .callback = gpt4_callback,
};

void motor_rotor_tracker_setup(void) {
  setup_hall_sensors();
  last_commutation_state = get_commutation_state();

  gptStart(&GPTD4, &gpt4cfg);
  gptStartContinuous(&GPTD4, 1);
}

float motor_rotor_tracker_position_revolution_percentage(void) {
  return remainder(last_position, 6) / 6.0;
}

float motor_rotor_tracker_velocity_revs_per_sec(void) {
  return remainder(last_velocity, 6) / 6.0;
}

float motor_rotor_tracker_acceleration_revs_per_second_squared(void) {
  return remainder(last_acceleration, 6) / 6.0;
}
