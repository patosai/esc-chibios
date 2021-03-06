#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

uint16_t util_format_str_with_newline(char *buffer, uint16_t total_buffer_size, const char *fmt, va_list ap);

float constrain(float x, float min, float max);
float scale(float x, float orig_min, float orig_max, float new_min, float new_max);

#endif
