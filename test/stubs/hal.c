#include <hal.h>

void gptStart(GPTDriver* driver, GPTConfig* config) {}
void gptStartContinuous(GPTDriver* driver, uint16_t frequency) {}

line_t PAL_LINE(port_t port, pad_t pad) {
  line_t line = {
    .port = port,
    .pad = pad
  };
  return line;
}
void palEnableLineEvent() {}
palstatus_t palReadLine(line_t line) {return PAL_LOW;}
void palSetLineCallback(line_t line, palcallback_t callback, void *arg) {}
void palSetLineEvent(line_t line, palcallback_t callback, void* arg) {}
void palSetLineMode(line_t line, uint8_t mode) {}
