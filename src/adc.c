#include <ch.h>
#include <hal.h>
#include <stdarg.h>

#include "adc.h"

// samples_saved must be 1 or an even number
#define ADC_SAMPLES_SAVED_PER_CHANNEL 1
#define ADC_1_CHANNELS 2
#define ADC_2_CHANNELS 2
#define ADC_3_CHANNELS 1

#define ADC_VOLTAGE_FACTOR (3.3 / 4095.0)
#define PHASE_RESISTANCE_OHMS 0.005
#define SAMPLE_TO_CURRENT_CONSTANT (ADC_VOLTAGE_FACTOR/PHASE_RESISTANCE_OHMS)

static adcsample_t adc1_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC_1_CHANNELS];
static adcsample_t adc2_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC_2_CHANNELS];
static adcsample_t adc3_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC_3_CHANNELS];

// STM32F4 on ChibiOS uses its ADCv2 driver, which does not support dual/triple mode ADCs
// Instead, just trigger all ADCs off the same timer

static const ADCConversionGroup adc1_config = {
  .circular = FALSE,
  .num_channels = ADC_1_CHANNELS,
  .end_cb = NULL,
  .error_cb = NULL,
  .cr1 = ADC_CR1_SCAN, // set scan mode so ADC will convert SQR1/2/3 channels
  // https://www.st.com/content/ccc/resource/technical/document/reference_manual/3d/6d/5a/66/b4/99/40/d4/DM00031020.pdf/files/DM00031020.pdf/jcr:content/translations/en.DM00031020.pdf
  // see section 13.6 Conversion on external trigger and trigger polarity
  .cr2 = ADC_CR2_ADON // turn ADC on
         | ADC_CR2_DMA // use DMA to transfer data to buffers
         | ADC_CR2_EXTEN_0 // trigger on rising edge
         | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1, // trigger on TIM2_TRGO
  .smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) | ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_56),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = ADC_SQR2_SQ8_N(ADC_CHANNEL_SENSOR),
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};

static const ADCConversionGroup adc2_config = {
  .circular = FALSE,
  .num_channels = ADC_2_CHANNELS,
  .end_cb = NULL,
  .error_cb = NULL,
  .cr1 = ADC_CR1_SCAN,
  .cr2 = ADC_CR2_ADON
         | ADC_CR2_DMA
         | ADC_CR2_EXTEN_0
         | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1,
  .smpr1 = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_56) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_56),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = ADC_SQR2_SQ8_N(ADC_CHANNEL_VREFINT),
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12)
};

static const ADCConversionGroup adc3_config = {
  .circular = FALSE,
  .num_channels = ADC_3_CHANNELS,
  .end_cb = NULL,
  .error_cb = NULL,
  .cr1 = ADC_CR1_SCAN,
  .cr2 = ADC_CR2_ADON
         | ADC_CR2_DMA
         | ADC_CR2_EXTEN_0
         | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1,
  .smpr1 = ADC_SMPR1_SMP_AN13(ADC_SAMPLE_56),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN13)
};

static const GPTConfig gpt_config = {
  .frequency = 100000U,
  .callback = NULL,
  // https://www.st.com/content/ccc/resource/technical/document/application_note/group0/91/01/84/3f/7c/67/41/3f/DM00236305/files/DM00236305.pdf/jcr:content/translations/en.DM00236305.pdf
  // see end of section 2.5
  // MMS[2:0] = 010 -> pulse on TRGO
  .cr2 = TIM_CR2_MMS_1,
  .dier = 0U
};

static void setup_pin_modes(void) {
  palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, 2, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG);
}

static void start_all_adcs(void) {
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD2, NULL);
  adcStart(&ADCD3, NULL);

  adcStartConversion(&ADCD1, &adc1_config, adc1_samples, ADC_SAMPLES_SAVED_PER_CHANNEL);
  adcStartConversion(&ADCD2, &adc2_config, adc2_samples, ADC_SAMPLES_SAVED_PER_CHANNEL);
  adcStartConversion(&ADCD3, &adc3_config, adc3_samples, ADC_SAMPLES_SAVED_PER_CHANNEL);
}

static void stop_all_adcs(void) {
  adcStopConversion(&ADCD3);
  adcStopConversion(&ADCD2);
  adcStopConversion(&ADCD1);

  adcStop(&ADCD3);
  adcStop(&ADCD2);
  adcStop(&ADCD1);
}

static void enable_temp_and_vref_sensors(void) {
  adcSTM32EnableTSVREFE();
}

static void disable_temp_and_vref_sensors(void) {
  adcSTM32DisableTSVREFE();
}

void adc_start_current_measurement_conversion(void) {
  setup_pin_modes();
  start_all_adcs();
  enable_temp_and_vref_sensors();
  gptStart(&GPTD2, &gpt_config);
}

void adc_stop_current_measurement_conversion(void) {
  gptStop(&GPTD2);
  disable_temp_and_vref_sensors();
  stop_all_adcs();
}

float adc_temp(void) {
  return adc1_samples[1] * ADC_VOLTAGE_FACTOR;
}

float adc_vref(void) {
  return adc2_samples[1] * ADC_VOLTAGE_FACTOR;
}

float adc_phase_a_current(void) {
  return adc1_samples[0] * SAMPLE_TO_CURRENT_CONSTANT;
}

float adc_phase_b_current(void) {
  return adc2_samples[0] * SAMPLE_TO_CURRENT_CONSTANT;
}

float adc_phase_c_current(void) {
  return adc3_samples[0] * SAMPLE_TO_CURRENT_CONSTANT;
}
