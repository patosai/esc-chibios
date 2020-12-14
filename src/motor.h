#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stdint.h"

void motor_init(void);
void motor_set_power_percentage(float);
void motor_get_phase_currents(float* buf);

float motor_rotor_position_radians(void);
void motor_update_routine(void);

#endif
