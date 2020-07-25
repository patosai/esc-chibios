#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stdint.h"

void motor_init(void);
void motor_set_power_percentage(uint8_t);
void motor_update_routine(void);

#endif
