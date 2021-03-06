#include <ch.h>
#include <hal.h>

#include "line.h"
#include "serial.h"
#include "util.h"

#define SERIAL_BAUD_RATE 38400
#define SERIAL_TOTAL_BUFFER_SIZE SERIAL_BUFFERS_SIZE

static char buffer[SERIAL_TOTAL_BUFFER_SIZE];
static const char error_preheader[] = "[ERROR] ";

static SerialConfig config = {
  .speed = SERIAL_BAUD_RATE,
  .cr1 = 0,
  .cr2 = USART_CR2_STOP1_BITS,
  .cr3 = 0
};

void serial1_init(void) {
  sdStart(&SD1, &config);
  palSetLineMode(LINE_SERIAL_TX, PAL_MODE_ALTERNATE(7));
  palSetLineMode(LINE_SERIAL_RX, PAL_MODE_ALTERNATE(7));
  palSetLineMode(LINE_SERIAL_CTS, PAL_MODE_ALTERNATE(7));
  palSetLineMode(LINE_SERIAL_RTS, PAL_MODE_ALTERNATE(7));
}

int serial1_println(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int formatted_bytes = util_format_str_with_newline(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  sdAsynchronousWrite(&SD1, (uint8_t*)buffer, formatted_bytes);
  va_end(ap);
  return formatted_bytes;
}

int serial1_println_sync(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int formatted_bytes = util_format_str_with_newline(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  sdWrite(&SD1, (uint8_t*)buffer, formatted_bytes);
  va_end(ap);
  return formatted_bytes;
}

void serial1_print_error_preheader(void) {
  sdWrite(&SD1, (uint8_t*)error_preheader, sizeof(error_preheader));
}

void serial1_print_error_preheader_in_interrupt(void) {
  chSysLockFromISR();
  sdWriteI(&SD1, (uint8_t*)error_preheader, sizeof(error_preheader));
  chSysUnlockFromISR();
}

int serial1_println_sync_in_interrupt(const char *fmt, ...) {
  chSysLockFromISR();
  va_list ap;
  va_start(ap, fmt);
  int formatted_bytes = util_format_str_with_newline(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  sdWriteI(&SD1, (uint8_t*)buffer, formatted_bytes);
  va_end(ap);
  chSysUnlockFromISR();
  return formatted_bytes;
}
