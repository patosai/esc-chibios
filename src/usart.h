#ifndef _USART_H_
#define _USART_H_

void serial_init(void);

int serial1_send(const char* fmt, ...);
int serial2_send(const char* fmt, ...);

#endif