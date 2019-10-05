#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stdint.h"

void motor_enable(void);
void motor_disable(void);

void motor_set_power_percentage(uint32_t power_percentage_0_to_10000);

#endif
