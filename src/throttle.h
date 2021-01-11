#ifndef _THROTTLE_H_
#define _THROTTLE_H_

#include <stdbool.h>

void throttle_init(void);
float throttle_percentage(void);
bool throttle_power_on(void);

#endif