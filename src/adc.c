#include <stdarg.h>

#include "ch.h"
#include "hal.h"

#include "adc.h"

// samples_saved be 1 or an even number
#define ADC_SAMPLES_SAVED_PER_CHANNEL 1
#define ADC_1_CHANNELS 3
#define ADC_2_CHANNELS 1

#define ADC_VOLTAGE_FACTOR (3.3 * ((2^12) - 1))

static adcsample_t phase_a_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC_1_CHANNELS];
static adcsample_t phase_b_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC_2_CHANNELS];

static ADCConversionGroup adc1Config = {
  .circular = TRUE,
  .num_channels = 3,
  .end_cb = NULL,
  .error_cb = NULL,
  .cr1 = 0,
  .cr2 = 0,
  .smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) | ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_144) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_144),
  .smpr2 = 0,
  .htr = 0,
  .ltr = 0,
  .sqr1 = 0,
  .sqr2 = ADC_SQR2_SQ8_N(ADC_CHANNEL_SENSOR) | ADC_SQR2_SQ7_N(ADC_CHANNEL_VREFINT),
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};

static ADCConversionGroup adc2Config = {
  .circular = TRUE,
  .num_channels = 1,
  .end_cb = NULL,
  .error_cb = NULL,
  .cr1 = 0,
  .cr2 = 0,
  .smpr1 = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_56),
  .smpr2 = 0,
  .htr = 0,
  .ltr = 0,
  .sqr1 = 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12)
};

static void setup_pin_modes(void) {
  palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, 2, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG);
}

static void start_adc_1_and_2(void) {
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD2, NULL);
}

static void stop_adc_1_and_2(void) {
  adcStop(&ADCD1);
  adcStop(&ADCD2);
}

static void enable_temp_and_vref_sensors(void) {
  adcSTM32EnableTSVREFE();
}

static void disable_temp_and_vref_sensors(void) {
  adcSTM32DisableTSVREFE();
}

void adc_start_current_measurement_conversion(void) {
  setup_pin_modes();
  start_adc_1_and_2();
  enable_temp_and_vref_sensors();
  adcStartConversion(&ADCD1, &adc1Config, phase_a_samples, ADC_SAMPLES_SAVED_PER_CHANNEL);
  adcStartConversion(&ADCD2, &adc2Config, phase_b_samples, ADC_SAMPLES_SAVED_PER_CHANNEL);
}

void adc_stop_current_measurement_conversion(void) {
  adcStopConversion(&ADCD1);
  adcStopConversion(&ADCD2);
  disable_temp_and_vref_sensors();
  stop_adc_1_and_2();
}

float adc_phase_a_voltage(void) {
  return phase_a_samples[0] * ADC_VOLTAGE_FACTOR;
}

float adc_phase_b_voltage(void) {
  return phase_b_samples[0] * ADC_VOLTAGE_FACTOR;
}

float adc_phase_c_voltage(void) {
  return -1 * (adc_phase_a_voltage() + adc_phase_b_voltage());
}
