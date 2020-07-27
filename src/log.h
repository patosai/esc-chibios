#ifndef _LOG_H_
#define _LOG_H_

void log_init(void);
void log_queue_message(const char* fmt, ...);
void log_queue_message_in_interrupt(const char* fmt, ...);
void log_print_queue_into_serial(void);

#endif