#ifndef _LOG_H_
#define _LOG_H_

#include "serial.h"

void log_init(void);
#define log_print(...) serial1_send(__VA_ARGS__)
void log_queue_message(const char* fmt, ...);
void log_queue_message_in_interrupt(const char* fmt, ...);
void log_print_queue_into_serial(void);

#endif