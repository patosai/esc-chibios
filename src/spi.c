#include "ch.h"
#include "hal.h"

#include "spi.h"

static void setup_pins(void) {
	palSetPadMode(GPIOB, 12, PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5));
}

static void start_spi(void) {
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
	uint16_t CR1 =
			SPI_CR1_CPHA // first data capture on second edge
			| SPI_CR1_DFF // 16-bit frame
			;
	uint16_t CR2 = 0; // no DMA, interrupts
	SPIConfig config = {
			.circular = true, // circular buffer, might need to also be enabled in halconf.h
			.end_cb = NULL, // complete callback
			.ssport = GPIOB, // chip select port
			.sspad = 12, // chip select pad
			.cr1 = CR1,
			.cr2 = CR2
	};
	spiStart(&SPID2, &config);
}

void spi2_init(void) {
	setup_pins();
	start_spi();
}

uint16_t spi2_exchange_synchronous(uint16_t n, uint16_t tx) {
	spiSelect(&SPID2);
	uint16_t txbuf[1] = {tx};
	// the rec length should be equal to the transmit length, so no buffer overflow should happen in rxbuf
	uint16_t rxbuf[1] = {0};
	spiExchange(&SPID2, n, txbuf, rxbuf);
	spiUnselect(&SPID2);
	return rxbuf[0];
}
