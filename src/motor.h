#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stdint.h"

void motor_pwm_init(void);
void motor_set_speed(uint16_t rpm);

#endif
