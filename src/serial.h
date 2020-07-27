#ifndef _SERIAL_H_
#define _SERIAL_H_

void serial1_init(void);
int serial1_send(const char* fmt, ...);
int serial1_send_sync(const char* fmt, ...);

#endif
