#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdarg.h>

void serial1_init(void);
int serial1_send(const char* fmt, ...);
int serial1_send_sync(const char* fmt, ...);
int serial1_send_sync_in_interrupt(const char *fmt, ...);

#endif
