#include <stdarg.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

#include "usart.h"

void usart1_init(void) {
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
  sdStart(&SD1, NULL);
}

int usart1_send(const char *fmt, ...) {
  va_list ap;
  int formatted_bytes;

  va_start(ap, fmt);
  formatted_bytes = chvprintf((BaseSequentialStream*)&SD1, fmt, ap);
  va_end(ap);

  return formatted_bytes;
}
