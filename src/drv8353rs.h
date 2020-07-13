#ifndef _DRV8353RS_H_
#define _DRV8353RS_H_

#include <ch.h>
#include <hal.h>
#include <stdbool.h>

typedef enum uint8_t {
  FAULT_STATUS_1 = 0x00,
  FAULT_STATUS_2,
  DRIVER_CONTROL,
  GATE_DRIVE_HIGH_CONTROL,
  GATE_DRIVE_LOW_CONTROL,
  OVERCURRENT_CONTROL,
  CURRENT_SENSE_CONTROL,
  DRIVER_CONFIGURATION
} drv8353rs_register_t;

void drv8353rs_init(void);
void drv8353rs_manually_calibrate(void);

bool drv8353rs_has_fault(void);
uint16_t drv8353rs_read_register(drv8353rs_register_t);

#endif
