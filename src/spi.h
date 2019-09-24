#ifndef _SPI_H_
#define _SPI_H_

#include "ch.h"
#include "hal.h"

void spi2_init(uint16_t cr1, uint16_t cr2);
uint16_t spi2_exchange_synchronous(uint16_t n, uint16_t txbuf);

#endif
