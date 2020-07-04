#include <ch.h>
#include <hal.h>

#include "spi.h"

static uint16_t SPI2_TRANSMIT_BUFFER[1];
static uint16_t SPI2_RECEIVE_BUFFER[1];

void spi2_init(uint16_t cr1, uint16_t cr2) {
  SPIConfig config = {
      .circular = false, // no circular buffer
      .end_cb = NULL, // no callback
      .ssport = GPIOB, // chip select port
      .sspad = 12, // chip select pad
      .cr1 = cr1,
      .cr2 = cr2
  };
  spiStart(&SPID2, &config);

  palSetPadMode(GPIOB, 12, PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5) | PAL_MODE_INPUT_PULLUP);
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5));
}

uint16_t spi2_exchange_word(uint16_t tx) {
  SPI2_TRANSMIT_BUFFER[0] = tx;
  spiSelect(&SPID2);
  spiExchange(&SPID2, 1, SPI2_TRANSMIT_BUFFER, SPI2_RECEIVE_BUFFER);
  spiUnselect(&SPID2);
  return SPI2_RECEIVE_BUFFER[0];
}