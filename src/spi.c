#include <ch.h>
#include <hal.h>

#include "spi.h"

// TODO after the flipped SDO/SDI wires for DRV8353RS are fixed on the PCB:
// remove the big banging and reenable the commented code which uses ChibiOS SPI drivers

/*
static SPIConfig *config;

static uint16_t SPI2_TRANSMIT_BUFFER[1];
static uint16_t SPI2_RECEIVE_BUFFER[1];
*/

void spi2_init(uint16_t cr1, uint16_t cr2) {
/*
  if (!config) {
    config = chHeapAlloc(NULL, sizeof(SPIConfig));
  }

  config->circular = false; // no circular buffer
  config->end_cb = NULL; // no callback
  config->ssport = GPIOB; // chip select port
  config->sspad = 12; // chip select pad
  config->cr1 = cr1;
  config->cr2 = cr2;
  spiStart(&SPID2, config);

  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL); // manual NSS
  palSetPad(GPIOB, 12);
  palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5)); // SCLK
  palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5)); // MISO
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5)); // MOSI
*/
  (void)cr1;
  (void)cr2;
  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL); // NSS
  palSetPad(GPIOB, 12);
  palSetPadMode(GPIOB, 13, PAL_MODE_OUTPUT_PUSHPULL); // SCLK
  palClearPad(GPIOB, 13);
  palSetPadMode(GPIOB, 14, PAL_MODE_OUTPUT_PUSHPULL); // MOSI
  palClearPad(GPIOB, 14);
  palSetPadMode(GPIOB, 15, PAL_MODE_INPUT_PULLUP); // MISO

  // configure auxiliary pins so I can use a logic analyzer to see what's going on
  palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_PUSHPULL); // NSS
  palSetPad(GPIOB, 5);
  palSetPadMode(GPIOB, 6, PAL_MODE_OUTPUT_PUSHPULL); // SCLK
  palClearPad(GPIOB, 6);
  palSetPadMode(GPIOD, 11, PAL_MODE_OUTPUT_PUSHPULL); // MOSI
  palClearPad(GPIOD, 11);
  palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL); // MISO
  palSetPad(GPIOD, 12);
}

uint16_t spi2_exchange_word(uint16_t tx) {
/*
  SPI2_TRANSMIT_BUFFER[0] = tx;

  spiSelect(&SPID2);
  palClearPad(GPIOB, 12);
  spiExchange(&SPID2, 1, SPI2_TRANSMIT_BUFFER, SPI2_RECEIVE_BUFFER);
  palSetPad(GPIOB, 12);
  spiUnselect(&SPID2);

  return SPI2_RECEIVE_BUFFER[0];
  */

  uint16_t data = 0;

  palClearPad(GPIOB, 12); // select the slave
  palClearPad(GPIOB, 5);
  chThdSleepMilliseconds(1);

  for (uint8_t ii = 0; ii < 16; ++ii) {
    palSetPad(GPIOB, 13); // clock goes high
    palSetPad(GPIOB, 6);
    chThdSleepMilliseconds(1);

    bool miso_is_high = palReadPad(GPIOB, 15) == PAL_HIGH;
    data = (data << 1) + miso_is_high; // read MISO
    if (miso_is_high) {
      palSetPad(GPIOD, 12);
    } else {
      palClearPad(GPIOD, 12);
    }
    chThdSleepMilliseconds(1);

    // set next bit of MOSI, MSB first
    bool next_bit = tx & (1 << (15 - ii));
    if (next_bit) {
      palSetPad(GPIOB, 14);
      palSetPad(GPIOD, 11);
    } else {
      palClearPad(GPIOB, 14);
      palClearPad(GPIOD, 11);
    }
    chThdSleepMilliseconds(1);

    // falling edge of clock
    palClearPad(GPIOB, 13);
    palClearPad(GPIOB, 6);
    chThdSleepMilliseconds(3);
  }

  palSetPad(GPIOB, 12); // unselect the slave
  palSetPad(GPIOB, 5);

  return data;
}
