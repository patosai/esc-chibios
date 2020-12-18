#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "util.h"

uint16_t util_format_str_with_newline(char *buffer, uint16_t total_buffer_size, const char *fmt, va_list ap) {
  // returns the number of bytes formatted into the buffer, excluding the ending null character
  uint16_t usable_buffer_size = total_buffer_size - 1; // accounts for null character at end

  int total_bytes = chvsnprintf(buffer, total_buffer_size, fmt, ap);
  chDbgAssert(total_bytes >= 0, "encoding error");

  // chvsnprintf returns the number of characters that would have been written if the buffer size had been sufficiently
  // large, not counting the terminating null character
  uint16_t formatted_bytes = total_bytes <= usable_buffer_size ? total_bytes : usable_buffer_size;
  // add newline (\r\n) to all strings
  bool can_fit_newline = formatted_bytes <= usable_buffer_size - 2;
  if (can_fit_newline) {
    buffer[formatted_bytes] = '\r';
    buffer[formatted_bytes+1] = '\n';
    buffer[formatted_bytes+2] = '\0';
    formatted_bytes += 2;
  } else {
    buffer[usable_buffer_size-2] = '\r';
    buffer[usable_buffer_size-1] = '\n';
    formatted_bytes = usable_buffer_size;
  }

  return formatted_bytes;
}

float constrain(float x, float min, float max) {
  if (x > max) {
    x = max;
  }
  if (x < min) {
    x = min;
  }
  return x;
}