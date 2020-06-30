#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <stdarg.h>

#include "serial.h"

#define SERIAL_BAUD_RATE 38400
#define SERIAL_TOTAL_BUFFER_SIZE SERIAL_BUFFERS_SIZE
#define SERIAL_USABLE_BUFFER_SIZE (SERIAL_TOTAL_BUFFER_SIZE-1) // accounts for null character at end

static char buffer[SERIAL_TOTAL_BUFFER_SIZE];

static SerialConfig config = {
  .speed = SERIAL_BAUD_RATE,
  .cr1 = 0,
  .cr2 = USART_CR2_STOP1_BITS,
  .cr3 = 0
};

void serial1_init(void) {
  sdStart(&SD1, &config);
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
}

static int format_buffer(const char *fmt, va_list ap) {
  int total_bytes = chvsnprintf(buffer, SERIAL_TOTAL_BUFFER_SIZE, fmt, ap);
  int string_bytes = total_bytes <= SERIAL_USABLE_BUFFER_SIZE ? total_bytes : SERIAL_USABLE_BUFFER_SIZE;
  // add newline (\r\n) to all strings
  bool overflowed = string_bytes > SERIAL_USABLE_BUFFER_SIZE - 2;
  if (overflowed) {
    buffer[SERIAL_USABLE_BUFFER_SIZE-2] = '\r';
    buffer[SERIAL_USABLE_BUFFER_SIZE-1] = '\n';
  } else {
    buffer[string_bytes] = '\r';
    buffer[string_bytes+1] = '\n';
    string_bytes += 2;
  }
  return string_bytes;
}

static int send_to_driver(SerialDriver* driver, const char *fmt, va_list ap) {
  int formatted_bytes = format_buffer(fmt, ap);
  sdWrite(driver, (uint8_t*)buffer, formatted_bytes);
  return formatted_bytes;
}

static int send_to_driver_async(SerialDriver* driver, const char *fmt, va_list ap) {
  int formatted_bytes = format_buffer(fmt, ap);
  sdAsynchronousWrite(driver, (uint8_t*)buffer, formatted_bytes);
  return formatted_bytes;
}

int serial1_send(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int formatted_bytes = send_to_driver(&SD1, fmt, ap);
  va_end(ap);

  return formatted_bytes;
}

int serial1_send_async(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int formatted_bytes = send_to_driver_async(&SD1, fmt, ap);
  va_end(ap);

  return formatted_bytes;
}
