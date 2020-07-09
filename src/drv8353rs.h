#ifndef _DRV8353RS_H_
#define _DRV8353RS_H_

#include <ch.h>
#include <hal.h>
#include <stdbool.h>

void drv8353rs_init(void);
void drv8353rs_manually_calibrate(void);

bool drv8353rs_has_fault(void);
uint16_t drv8353rs_read_register(uint8_t);

#endif
