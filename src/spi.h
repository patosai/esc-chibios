#ifndef _SPI_H_
#define _SPI_H_

#include <ch.h>
#include <hal.h>

void spi2_init(void);
uint16_t spi2_exchange_uint16(uint16_t txbuf);

#endif
