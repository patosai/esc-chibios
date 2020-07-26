#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>

void log_queue_message(const char* fmt, va_list ap);
char* log_queue_get_message(void);

#endif