#ifndef _PID_H_
#define _PID_H_

typedef struct {
  float p;
  float i;
  float d;
  float min_output;
  float max_output;
  float last_error;
  float accumulated_error;
} pid_state_t;

pid_state_t pid_create(float p, float i, float d, float min_output, float max_output);
void pid_reset(pid_state_t *pid);
float pid_update(pid_state_t *pid, float expected, float actual);

#endif
