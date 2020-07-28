#ifndef _LOG_H_
#define _LOG_H_

#include "serial.h"

#define log_init() serial1_init()
#define log_println(...) serial1_send(__VA_ARGS__)
#define log_println_in_interrupt(...) serial1_send_sync_in_interrupt(__VA_ARGS__)

#endif