#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "ch.h"
#include "hal.h"

void spi2_init(void);
void spi2_send(uint16_t n, uint8_t* buf);

#endif