#include "pid.h"

pid_state_t pid_create(float p, float i, float d, float min_output, float max_output) {
  pid_state_t ret = {
    .p = p,
    .i = i,
    .d = d,
    .min_output = min_output,
    .max_output = max_output,
    .last_error = 0,
    .accumulated_error = 0
  };
  return ret;
}

void pid_reset(pid_state_t *pid) {
  pid->last_error = 0;
  pid->accumulated_error = 0;
}

float pid_update(pid_state_t *pid, float expected, float actual) {
  float error = expected - actual;
  pid->accumulated_error += error;
  float output = (pid->p * error) + (pid->i * pid->accumulated_error) + (pid->d * (pid->last_error - error));
  pid->last_error = error;

  // stop integral component runaway
  if (pid->accumulated_error > pid->max_output) {
    pid->accumulated_error = pid->max_output;
  } else if (pid->accumulated_error < pid->min_output) {
    pid->accumulated_error = pid->min_output;
  }

  // constrain output
  if (output > pid->max_output) {
    output = pid->max_output;
  } else if (output < pid->min_output) {
    output = pid->min_output;
  }
  return output;
}
