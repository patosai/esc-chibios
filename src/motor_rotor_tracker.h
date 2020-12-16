#ifndef _MOTOR_ROTOR_TRACKER_H_
#define _MOTOR_ROTOR_TRACKER_H_

void motor_rotor_tracker_setup(void);
float motor_rotor_tracker_position_revolution_fraction(void);
float motor_rotor_tracker_velocity_revs_per_sec(void);
float motor_rotor_tracker_acceleration_revs_per_second_squared(void);

#endif
