#ifndef _THROTTLE_H_
#define _THROTTLE_H_

#include <stdbool.h>

void throttle_init(void);
bool throttle_power_on(void);

#define throttle_percentage() adc_throttle_percentage()

#endif