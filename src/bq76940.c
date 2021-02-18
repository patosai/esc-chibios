#include <ch.h>
#include <hal.h>

#include "bq76940.h"
#include "i2c.h"

static uint8_t txbuf[1];
static uint8_t rxbuf[1];

void bq76940_init(void) {
  i2c_init();

  // protect1
  // ideal short circuit threshold = 2mOhm * 40A = 80mV
  bq76940_send(
    PROTECT1,
    (1 << 7) | // RSNS = 1
      (0b01 << 3) | // 100us delay
      (0x02 << 0) // with RSNS = 1, SCD = 89mV
  );

  // protect2
  // ideal overcurrent threshold = 2mOhm * 30A = 60mV
  bq76940_send(
    PROTECT2,
    (0x05 << 4) | // 320ms delay
      (0x08 << 0) // with RSNS = 1, OCD = 61mV
  );

  // protect3
  bq76940_send(
    PROTECT3,
    (0x1 << 6) | // 4s undervoltage delay
      (0x01 << 4) // 2s overvoltage delay
  );

  // ov_trip
  float desired_ov_voltage = 4.25;
  uint16_t adc_value = (desired_ov_voltage*1e6 - bq76940_adcoffset()) / (bq76940_adcgain());
  uint8_t register_value = (adc_value >> 4) & 0xFF;
  bq76940_send(
    OV_TRIP,
    register_value
  );

  // uv_trip
  // maximum UV voltage is 3.1V, so set it to that
  bq76940_send(
    OV_TRIP,
    0b11111111
  );
}

void bq76940_send(bq76940_register_t addr, uint8_t data) {
  txbuf[0] = data;
  i2c_send_no_receive(addr, txbuf, 1);
}

uint8_t bq76940_read(bq76940_register_t addr) {
  i2c_send(addr, txbuf, 0, rxbuf, 1);
  return rxbuf[0];
}

uint8_t bq76940_adcgain(void) {
  uint8_t upper_bits = (bq76940_read(ADCGAIN1) & 0b1100) << 1;
  uint8_t lower_bits = (bq76940_read(ADCGAIN2) & 0b11100000) >> 5;
  uint8_t result =  upper_bits | lower_bits;
  return result + 365;
}

int bq76940_adcoffset(void) {
  uint8_t result = bq76940_read(ADCOFFSET);
  return result - 128;
}