#include <stdarg.h>

#include "ch.h"
#include "hal.h"

#include "adc.h"

#define ADC_SAMPLES_SAVED_PER_CHANNEL 16
#define ADC_NUM_CHANNELS 3

static adcsample_t samples_buf[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC_NUM_CHANNELS];

//static ADCConversionGroup adc1Config = {};
//adc1Config.circular = true;
//adc1Config.num_channels = 1;
//adc1Config.end_cb = NULL;
//adc1Config.error_cb = NULL;
//adc1Config.cr1 = 0;
//adc1Config.cr2 = 0;
//adc1Config.smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) | ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_144) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_144);
//adc1Config.smpr2 = 0;
//adc1Config.htr = 0;
//adc1Config.ltr = 0;
//adc1Config.sqr1 = 0;
//adc1Config.sqr2 = ADC_SQR2_SQ8_N(ADC_CHANNEL_SENSOR) | ADC_SQR2_SQ7_N(ADC_CHANNEL_VREFINT);
//adc1Config.sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11);

static ADCConversionGroup adc1Config = {
  TRUE, // circular buffer
  1, // number of channels
  NULL, // callback
  NULL, // error callback
  0, // CR1
  0, // CR2
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) | ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_144) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_144), // SMPR1
  0, // SMPR2
  0, // HTR
  0, // LTR
  0, // SQR1
  ADC_SQR2_SQ8_N(ADC_CHANNEL_SENSOR) | ADC_SQR2_SQ7_N(ADC_CHANNEL_VREFINT), // SQR2
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) // SQR3
};

static const ADCConversionGroup adc2Config = {
  TRUE, // circular buffer
  1, // number of channels
  NULL, // callback
  NULL, // error callback
  0, // CR1
  0, // CR2
  ADC_SMPR1_SMP_AN12(ADC_SAMPLE_56), // SMPR1
  0, // SMPR2
  0, // HTR
  0, // LTR
  0, // SQR1
  0, // SQR2
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12) // SQR3
};

static const ADCConversionGroup adc2Config = {
  TRUE, // circular buffer
  1, // number of channels
  NULL, // callback
  NULL, // error callback
  0, // CR1
  0, // CR2
  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_56), // SMPR1
  0, // SMPR2
  0, // HTR
  0, // LTR
  0, // SQR1
  0, // SQR2
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN13) // SQR3
};

void adc_init(void) {
  palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, 2, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG);
}
