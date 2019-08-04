#include "ch.h"
#include "hal.h"

#include "serial.h"

void serial_init(void) {
  uint16_t CR1 = 1 << 0 // first clock transition is the first data capture edge
                 | 1 << 1 // clock is 0 when idle
                 | 1 << 2 // master mode
                 | 3 << 3 // baud rate = fPCLK/16
                 | 1 << 6 // SPI enabled
                 ;
  uint16_t CR2 = 0; // no DMA, interrupts

  palSetPadMode(GPIOB, 12, PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5));

  SPIConfig config = {
    false, // circular buffer, might need to also be enabled in halconf.h
    NULL, // complete callback
    GPIOB, // chip select port
    12, // chip select pad
    CR1,
    CR2
  };
  spiStart(&SPID2, &config);
}
