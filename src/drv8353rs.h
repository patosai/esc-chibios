#ifndef _DRV8353RS_H_
#define _DRV8353RS_H_

#include "stdbool.h"

void drv8353rs_init(void);
void drv8353rs_manually_calibrate(void);

bool has_fault(void);

#endif
