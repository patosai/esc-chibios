#include <ch.h>
#include <hal.h>

#include "i2c.h"
#include "line.h"
#include "log.h"

const sysinterval_t timeout_ticks = 100;

static const I2CConfig config = {
  .op_mode = OPMODE_I2C,
  .clock_speed = 100000,
  .duty_cycle = STD_DUTY_CYCLE
};

void i2c_init(void) {
  palSetLineMode(LINE_I2C_SCL, PAL_MODE_ALTERNATE(4));
  palSetLineMode(LINE_I2C_SDA, PAL_MODE_ALTERNATE(4));
  i2cStart(&I2CD2, &config);
}

void i2c_send(uint8_t addr, const uint8_t *txbuf, const size_t txlen, uint8_t *rxbuf, const size_t rxlen) {
  i2cAcquireBus(&I2CD2);
  msg_t status = i2cMasterTransmitTimeout(
    &I2CD2,
    addr,
    txbuf,
    txlen,
    rxbuf,
    rxlen,
    timeout_ticks
  );
  i2cReleaseBus(&I2CD2);
  if (status == MSG_RESET) {
    i2cflags_t flags = i2cGetErrors(&I2CD2);
    log_error("I2C2 error %d", flags);
  }
}

void i2c_send_no_receive(uint8_t addr, const uint8_t *txbuf, const size_t txlen) {
  i2c_send(addr, txbuf, txlen, NULL, 0);
}
