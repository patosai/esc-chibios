#ifndef _I2C_H_
#define _I2C_H_

void i2c_init(void);
void i2c_send(uint8_t addr, const uint8_t *txbuf, const size_t txlen, uint8_t *rxbuf, const size_t rxlen);
void i2c_send_no_receive(uint8_t addr, const uint8_t *txbuf, const size_t txlen);

#endif
