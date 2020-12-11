#include <ch.h>
#include <hal.h>

#include "line.h"
#include "spi.h"

static SPIConfig *config;

static uint16_t SPI_RECEIVE_BUFFER[1];

void spi2_init(uint16_t cr1, uint16_t cr2) {
  // set NSS line manually
  palSetLineMode(LINE_SPI_NSS, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLine(LINE_SPI_NSS);
  palSetLineMode(LINE_SPI_SCLK, PAL_MODE_ALTERNATE(5));
  // MISO needs special input pullup... didn't work for me using PAL_MODE_ALTERNATE(5)
  palSetLineMode(LINE_SPI_MISO, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(5) | PAL_STM32_PUPDR_PULLUP);
  palSetLineMode(LINE_SPI_MOSI, PAL_MODE_ALTERNATE(5));

  if (!config) {
    config = chHeapAlloc(NULL, sizeof(SPIConfig));
  }

  config->circular = false; // no circular buffer
  config->end_cb = NULL; // no callback
  config->ssline = LINE_SPI_NSS; // chip select line
  config->cr1 = cr1;
  config->cr2 = cr2;
  spiStart(&SPID2, config);
}

uint16_t spi2_exchange_word(const uint16_t tx) {
  palClearLine(LINE_SPI_NSS);
  spiSelect(&SPID2);
  spiExchange(&SPID2, 1, &tx, SPI_RECEIVE_BUFFER);
  spiUnselect(&SPID2);
  palSetLine(LINE_SPI_NSS);
  return SPI_RECEIVE_BUFFER[0];
}
