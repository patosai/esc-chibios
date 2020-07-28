#include <ch.h>
#include <hal.h>
#include <stdarg.h>

#include "log.h"
#include "util.h"

#define LOG_MESSAGE_BUFFER_SIZE 16
#define LOG_STRING_MAX_LENGTH 256

static objects_fifo_t message_fifo;
static msg_t msg_buffer[LOG_MESSAGE_BUFFER_SIZE];
static char string_buffer[LOG_MESSAGE_BUFFER_SIZE][LOG_STRING_MAX_LENGTH];

void log_init(void) {
  serial1_init();
  chFifoObjectInit(&message_fifo, LOG_STRING_MAX_LENGTH, LOG_MESSAGE_BUFFER_SIZE, string_buffer, msg_buffer);
}

void log_queue_message(const char* fmt, ...) {
  char *msg_buf = chFifoTakeObjectTimeout(&message_fifo, TIME_INFINITE);
  chDbgAssert(msg_buf != NULL, "can't add log queue message");

  va_list ap;
  va_start(ap, fmt);
  util_format_str_with_newline(msg_buf, LOG_STRING_MAX_LENGTH, fmt, ap);
  va_end(ap);

  chFifoSendObject(&message_fifo, msg_buf);
}

void log_queue_message_in_interrupt(const char *fmt, ...) {
  chSysLockFromISR();
  char *msg_buf = chFifoTakeObjectI(&message_fifo);
  chDbgAssert(msg_buf != NULL, "can't add log queue message");

  va_list ap;
  va_start(ap, fmt);
  util_format_str_with_newline(msg_buf, LOG_STRING_MAX_LENGTH, fmt, ap);
  va_end(ap);

  chFifoSendObjectI(&message_fifo, msg_buf);
  chSysUnlockFromISR();
}

void log_print_queue_into_serial(void) {
  char *str;
  msg_t msg = chFifoReceiveObjectTimeout(&message_fifo, (void**)&str, TIME_INFINITE);
  while (msg == MSG_OK) {
    serial1_send_sync(str);
    chFifoReturnObject(&message_fifo, str);
    msg = chFifoReceiveObjectTimeout(&message_fifo, (void**)&str, TIME_INFINITE);
  }
}
