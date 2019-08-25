#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "ch.h"
#include "hal.h"

void spi_init(void);
void spi_send(uint16_t n, uint8_t* buf);

#endif