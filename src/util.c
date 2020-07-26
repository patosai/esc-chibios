#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "util.h"

int util_format_buffer(char *buffer, uint16_t total_buffer_size, const char *fmt, va_list ap) {
  uint16_t usable_buffer_size = total_buffer_size - 1; // accounts for null character at end

  int total_bytes = chvsnprintf(buffer, total_buffer_size, fmt, ap);
  int string_bytes = total_bytes <= usable_buffer_size ? total_bytes : usable_buffer_size;
  // add newline (\r\n) to all strings
  bool overflowed = string_bytes > usable_buffer_size - 2;
  if (overflowed) {
    buffer[usable_buffer_size-2] = '\r';
    buffer[usable_buffer_size-1] = '\n';
  } else {
    buffer[string_bytes] = '\r';
    buffer[string_bytes+1] = '\n';
    string_bytes += 2;
  }
  return string_bytes;
}

