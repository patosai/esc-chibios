#ifndef _TEST_STUB_CHPRINTF_H_
#define _TEST_STUB_CHPRINTF_H_

#include <stdarg.h>
#include <stdint.h>

int chvsnprintf(char *buffer, uint16_t total_buffer_size, const char *fmt, va_list ap);

#endif