#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "ch.h"
#include "hal.h"

void spi2_init(void);
uint16_t spi2_exchange_synchronous(uint16_t n, uint16_t txbuf);

#endif
