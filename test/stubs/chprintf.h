#ifndef _TEST_STUB_CHPRINTF_H_
#define _TEST_STUB_CHPRINTF_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#define chvsnprintf(buf, size, fmt, ap) vsnprintf(buf, size, fmt, ap)

#endif