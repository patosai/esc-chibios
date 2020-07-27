#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdarg.h>

uint16_t util_format_str_with_newline(char *buffer, uint16_t total_buffer_size, const char *fmt, va_list ap);

#endif
