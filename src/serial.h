#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdarg.h>

void serial1_init(void);
int serial1_println(const char* fmt, ...);
int serial1_println_sync(const char* fmt, ...);
int serial1_println_sync_in_interrupt(const char *fmt, ...);
void serial1_print_error_preheader(void);
void serial1_print_error_preheader_in_interrupt(void);

#endif
