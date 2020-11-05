#include <ch.h>
#include <hal.h>

#include "serial.h"
#include "util.h"

#define SERIAL_BAUD_RATE 38400
#define SERIAL_TOTAL_BUFFER_SIZE SERIAL_BUFFERS_SIZE

static char buffer[SERIAL_TOTAL_BUFFER_SIZE];

static SerialConfig config = {
  .speed = SERIAL_BAUD_RATE,
  .cr1 = 0,
  .cr2 = USART_CR2_STOP1_BITS,
  .cr3 = 0
};

void serial1_init(void) {
  sdStart(&SD1, &config);
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // TX
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // RX
  palSetPadMode(GPIOA, 11, PAL_MODE_ALTERNATE(7)); // CTS
  palSetPadMode(GPIOA, 12, PAL_MODE_ALTERNATE(7)); // RTS
}

int serial1_send(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int formatted_bytes = util_format_str_with_newline(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  sdAsynchronousWrite(&SD1, (uint8_t*)buffer, formatted_bytes);
  va_end(ap);

  return formatted_bytes;
}

int serial1_send_sync(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int formatted_bytes = util_format_str_with_newline(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  sdWrite(&SD1, (uint8_t*)buffer, formatted_bytes);
  va_end(ap);

  return formatted_bytes;
}

int serial1_send_sync_in_interrupt(const char *fmt, ...) {
  chSysLockFromISR();
  va_list ap;
  va_start(ap, fmt);
  int formatted_bytes = util_format_str_with_newline(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  sdWriteI(&SD1, (uint8_t*)buffer, formatted_bytes);
  va_end(ap);
  chSysUnlockFromISR();

  return formatted_bytes;
}
