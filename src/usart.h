#ifndef _USART_H_
#define _USART_H_

void usart1_init(void);

int usart1_send(const char* fmt, ...);

#endif