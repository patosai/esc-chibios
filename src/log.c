#include <ch.h>
#include <hal.h>

#include "log.h"
#include "util.h"

static char message_buffer[10][1024];

void log_queue_message(const char* fmt, va_list ap) {
  // TODO
  util_format_buffer(message_buffer[0], 1024, fmt, ap);
}

char* log_queue_get_message(void) {
  // TODO
  return message_buffer[0];
}
