#ifndef _LOG_H_
#define _LOG_H_

#include "serial.h"

#define log_init() serial1_init()
#define log_println(...) serial1_println(__VA_ARGS__)
#define log_error(...) {\
  serial1_print_error_preheader();\
  serial1_println(__VA_ARGS__);\
}
#define log_println_in_interrupt(...) serial1_println_sync_in_interrupt(__VA_ARGS__)
#define log_error_in_interrupt(...) {\
  serial1_print_error_preheader_in_interrupt();\
  serial1_println_sync_in_interrupt(__VA_ARGS__);\
}

#endif