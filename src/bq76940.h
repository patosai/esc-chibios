#ifndef _BQ76940_H_
#define _BQ76940_H_

typedef enum uint8_t {
  PROTECT1 = 0x06,
  PROTECT2 = 0x07,
  PROTECT3 = 0x08,
  OV_TRIP = 0x09,
  UV_TRIP = 0x0A,
  ADCGAIN1 = 0x50,
  ADCOFFSET = 0x51,
  ADCGAIN2 = 0x59,

} bq76940_register_t;

void bq76940_init(void);
void bq76940_send(bq76940_register_t, uint8_t data);
uint8_t bq76940_read(bq76940_register_t addr);

uint8_t bq76940_adcgain(void);
int bq76940_adcoffset(void);

#endif
