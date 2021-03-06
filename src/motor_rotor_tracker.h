#ifndef _MOTOR_ROTOR_TRACKER_H_
#define _MOTOR_ROTOR_TRACKER_H_

#include <stdint.h>

void motor_rotor_tracker_setup(void);
uint8_t motor_rotor_tracker_last_commutation_state(void);
float motor_rotor_tracker_position_revolution_percentage(void);
float motor_rotor_tracker_velocity_revs_per_sec(void);
float motor_rotor_tracker_acceleration_revs_per_second_squared(void);

#endif
