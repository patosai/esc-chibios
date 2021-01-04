#ifndef _TEST_STUB_HAL_H_
#define _TEST_STUB_HAL_H_

#include <stdint.h>

#define NULL 0

typedef struct {
  uint16_t frequency;
  void *callback;
} GPTConfig;

typedef struct {
} GPTDriver;

GPTDriver GPTD4;

void gptStart(GPTDriver* driver, GPTConfig* config);
void gptStartContinuous(GPTDriver* driver, uint16_t frequency);

// LINES
typedef uint8_t port_t;
typedef uint8_t pad_t;
#define GPIOA 0
#define GPIOB 1
#define GPIOC 2
#define GPIOD 3
#define GPIOE 4

#define GPIOA_PIN1 1
#define GPIOA_PIN2 2
#define GPIOA_PIN3 3
typedef struct {
  port_t port;
  pad_t pad;
} line_t;
line_t PAL_LINE(port_t, pad_t);

// PAL
#define PAL_EVENT_MODE_BOTH_EDGES 5

typedef void* palcallback_t;
typedef uint8_t palstatus_t;
#define PAL_LOW 0
#define PAL_HIGH 0
#define PAL_MODE_INPUT_PULLUP 0
palstatus_t palReadLine(line_t line);
void palEnableLineEvent();
void palSetLineCallback(line_t line, palcallback_t callback, void *arg);
void palSetLineMode(line_t line, uint8_t mode);
void palSetLineEvent(line_t line, palcallback_t callback, void* arg);

#endif