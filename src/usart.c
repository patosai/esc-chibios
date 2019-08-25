#include <stdarg.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

#include "usart.h"

#define USART_BUFFER_SIZE 1024

static char buffer[USART_BUFFER_SIZE];

static SerialConfig config = {
  38400,
  0,
  USART_CR2_STOP1_BITS,
  0
};

static void serial1_init(void) {
  sdStart(&SD1, &config);
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
}

static void serial2_init(void) {
  sdStart(&SD2, &config);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
}

void serial_init(void) {
  serial1_init();
  serial2_init();
}

static int serial_send(SerialDriver* driver, const char *fmt, va_list ap) {
  int formatted_bytes = chvsnprintf(buffer, USART_BUFFER_SIZE, fmt, ap);
  sdWrite(driver, (uint8_t*)buffer, formatted_bytes);
  return formatted_bytes;
}

int serial1_send(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int formatted_bytes = serial_send(&SD1, fmt, ap);
  va_end(ap);

  return formatted_bytes;
}

int serial2_send(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int formatted_bytes = serial_send(&SD2, fmt, ap);
  va_end(ap);

  return formatted_bytes;
}
