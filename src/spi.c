#include <ch.h>
#include <hal.h>

#include "spi.h"

static void setup_pins(void) {
	palSetPadMode(GPIOB, 12, PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5));
	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5));
}

void spi2_init(uint16_t cr1, uint16_t cr2) {
	setup_pins();
	SPIConfig config = {
			.circular = true, // circular buffer, might need to also be enabled in halconf.h
			.end_cb = NULL, // complete callback
			.ssport = GPIOB, // chip select port
			.sspad = 12, // chip select pad
			.cr1 = cr1,
			.cr2 = cr2
	};
	spiStart(&SPID2, &config);
}

uint16_t spi2_exchange_sync(uint16_t n, uint16_t tx) {
	spiSelect(&SPID2);
	uint16_t txbuf[1] = {tx};
	// the rec length should be equal to the transmit length, so no buffer overflow should happen in rxbuf
	uint16_t rxbuf[1] = {0};
	spiExchange(&SPID2, n, txbuf, rxbuf);
	spiUnselect(&SPID2);
	return rxbuf[0];
}
