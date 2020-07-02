#include <ch.h>
#include <hal.h>

#include "spi.h"

static uint16_t SPI2_TRANSMIT_BUFFER[1];
static uint16_t SPI2_RECEIVE_BUFFER[1];

static void setup_pins(void) {
  palSetPadMode(GPIOB, 12, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
}

void spi2_init(void) {
  setup_pins();

  // https://www.st.com/content/ccc/resource/technical/document/reference_manual/3d/6d/5a/66/b4/99/40/d4/DM00031020.pdf/files/DM00031020.pdf/jcr:content/translations/en.DM00031020.pdf
  // section 28.5 SPI and I2S registers

  // DRV8353RS
  // http://www.ti.com/lit/ds/symlink/drv8353.pdf
  // section 8.5.1.1
  // The SCLK pin should be low when the nSCS pin transitions from high to low and from low to high.
  //   ->> CPOL = 1
  // Data is captured on the falling edge of SCLK and data is propagated on the rising edge of SCLK.
  //  -> data is propagated on the rising edge, so capture should happen on the falling edge
  //  -> since CPOL is 0, the falling edge is the second clock
  //  ->> CPHA = 1
  uint16_t cr1 =
      SPI_CR1_CPHA // first data capture on second edge
      | SPI_CR1_DFF // 16-bit frame
      ;
  uint16_t cr2 = 0; // no DMA, interrupts

  SPIConfig config = {
      .circular = true, // no circular buffer
      .end_cb = NULL, // no callback
      .ssport = GPIOB, // chip select port
      .sspad = 12, // chip select pad
      .cr1 = cr1,
      .cr2 = cr2
  };
  spiStart(&SPID2, &config);
}

uint16_t spi2_exchange_uint16(uint16_t tx) {
  SPI2_TRANSMIT_BUFFER[0] = tx;
  spiSelect(&SPID2);
  spiExchange(&SPID2, 1, SPI2_TRANSMIT_BUFFER, SPI2_RECEIVE_BUFFER);
  spiUnselect(&SPID2);
  return SPI2_RECEIVE_BUFFER[0];
}
