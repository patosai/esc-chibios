#include <ch.h>
#include <hal.h>

#include "adc.h"
#include "line.h"
#include "log.h"

// On the STM32F407, all ADCs are run off of APB2
// Last time I checked mcuconf.h, APB2 runs at 42MHz
// ADCPRE divider is set at APB2 / 4, so ADC clock is 10.5MHz

// samples_saved must be 1 or an even number
#define ADC_SAMPLES_SAVED_PER_CHANNEL 1
#define ADC1_CHANNELS 2
#define ADC2_CHANNELS 2
#define ADC3_CHANNELS 1

#define ADC_VOLTAGE_FACTOR (3.3 / 4095.0)

#define ADC_TRIGGER_FREQUENCY 3500 // trigger frequency of ADCs (Hz)
#define GPT2_TIMER_FREQUENCY 21000 // timer frequency, must be a multiple of ADC_TRIGGER_FREQUENCY, and must evenly divide APB1 = 21MHz
// it seems 10kHz works but system crashes at 1Khz, perhaps timer upper limit?

static adcsample_t adc1_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC1_CHANNELS];
static adcsample_t adc2_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC2_CHANNELS];
static adcsample_t adc3_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC3_CHANNELS];

static float buffered_current_sense_voltages[ADC_MOTOR_PHASES_SAMPLED];

static uint8_t adcdriver_to_num(ADCDriver* adc) {
  if (adc->adc == ADC1) {
    return 1;
  } else if (adc->adc == ADC2) {
    return 2;
  } else if (adc->adc == ADC3)  {
    return 3;
  }
  chDbgAssert(false, "unknown ADC");
  return 0;
}

static void all_adcs_converted_callback(ADCDriver* adc) {
  // using the MULTI trigger mode, this callback is called when all ADCs have sampled/converted
  // should only be enabled on one of the ADCs, otherwise it will be called multiple times
  (void)adc;
  chSysLockFromISR();
  buffered_current_sense_voltages[0] = adc2_samples[0];
  buffered_current_sense_voltages[1] = adc3_samples[0];
  chSysUnlockFromISR();
}

static void adc_common_error_callback(ADCDriver *adc, adcerror_t err) {
  log_println_in_interrupt("ADC %d error %d", adcdriver_to_num(adc), err);
}

// ADC 1 samples VREF, and temp
static const ADCConversionGroup adc1_config = {
  // circular buffer needs to be enabled for ADC triggering to work
  .circular = TRUE,
  .num_channels = ADC1_CHANNELS,
  .end_cb = all_adcs_converted_callback,
  .error_cb = adc_common_error_callback,
  .cr1 = ADC_CR1_SCAN, // set scan mode so ADC will convert SQR1/2/3 channels
  // https://www.st.com/content/ccc/resource/technical/document/reference_manual/3d/6d/5a/66/b4/99/40/d4/DM00031020.pdf/files/DM00031020.pdf/jcr:content/translations/en.DM00031020.pdf
  // see section 13.6 Conversion on external trigger and trigger polarity
  .cr2 = ADC_CR2_EXTEN_RISING // trigger on rising edge
         | ADC_CR2_EXTSEL_SRC(6), // trigger on TIM2_TRGO
  // ADCCLK operates at APB2/4 = 42MHz/4 = 10.5MHz
  // Temp and Vref need a min. sampling time of 10us = 105 cycles needed
  // at 3.5kHz trigger, with the clock at 10.5MHz, ADC has 3000 cycles to do one conversion sequence
  // so sample the max - 480 cycles
  .smpr1 = ADC_SMPR1_SMP_VREF(ADC_SAMPLE_480) | ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_VREFINT) | ADC_SQR3_SQ2_N(ADC_CHANNEL_SENSOR),
};

// ADC 2 samples PC3 (phase B voltage) and PA6 (throttle)
static const ADCConversionGroup adc2_config = {
  .circular = TRUE,
  .num_channels = ADC2_CHANNELS,
  .end_cb = NULL,
  .error_cb = adc_common_error_callback,
  .cr1 = ADC_CR1_SCAN,
  .cr2 = ADC_CR2_EXTEN_RISING, // no external src set; triggered directly from ADC1 through ADC_CCR_MULTI
  .smpr1 = ADC_SMPR1_SMP_AN13(ADC_SAMPLE_480),
  .smpr2 = ADC_SMPR2_SMP_AN6(ADC_SAMPLE_480),
  .sqr1 = 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN13) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN6),
};

// ADC 3 samples PC2 (phase C voltage)
static const ADCConversionGroup adc3_config = {
  .circular = TRUE,
  .num_channels = ADC3_CHANNELS,
  .end_cb = NULL,
  .error_cb = adc_common_error_callback,
  .cr1 = ADC_CR1_SCAN,
  .cr2 = ADC_CR2_EXTEN_RISING, // no external src set; triggered directly from ADC1 through ADC_CCR_MULTI
  .smpr1 = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_480),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12),
};

static void setup_pin_modes(void) {
  palSetLineMode(LINE_THROTTLE_INPUT, PAL_MODE_INPUT_ANALOG);
  palSetLineMode(LINE_PHASE_A_VSENSE, PAL_MODE_INPUT_ANALOG);
  palSetLineMode(LINE_PHASE_B_VSENSE, PAL_MODE_INPUT_ANALOG);
  palSetLineMode(LINE_PHASE_C_VSENSE, PAL_MODE_INPUT_ANALOG);
}

static void start_all_adcs(void) {
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD2, NULL);
  adcStart(&ADCD3, NULL);

  adcSTM32EnableTSVREFE();

  // STM32F4 uses the ChibiOS ADCv2 driver, which does not support dual/triple mode ADCs natively
  // Instead, write to the ADC_CCR MULTI register manually
  // 10110 is triple mode, regular simultaneous mode conversion
  ADC->CCR = (ADC->CCR | ADC_CCR_MULTI_4 | ADC_CCR_MULTI_2 | ADC_CCR_MULTI_1) & ~ADC_CCR_MULTI_3 & ~ADC_CCR_MULTI_0;

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

static const GPTConfig gpt2_config = {
  .frequency = GPT2_TIMER_FREQUENCY,
  .callback = NULL,
  // https://www.st.com/content/ccc/resource/technical/document/application_note/group0/91/01/84/3f/7c/67/41/3f/DM00236305/files/DM00236305.pdf/jcr:content/translations/en.DM00236305.pdf
  // see end of section 2.5
  // MMS[2:0] = 010 -> pulse on TRGO
  .cr2 = TIM_CR2_MMS_1,
  .dier = 0U
};

static void start_timer(void) {
  gptStart(&GPTD2, &gpt2_config);
  gptStartContinuous(&GPTD2, GPT2_TIMER_FREQUENCY / ADC_TRIGGER_FREQUENCY);
}

static void stop_timer(void) {
  gptStopTimer(&GPTD2);
  gptStop(&GPTD2);
}

void adc_start_continuous_conversion(void) {
  setup_pin_modes();
  start_all_adcs();
  start_timer();
}

void adc_stop_continuous_conversion(void) {
  stop_timer();
  stop_all_adcs();
}

float adc_temp_celsius(void) {
  // formula from reference manual section 13.10 subsection "Reading the temperature"
  // constant values from F407VG datasheet
  const float V_25 = 0.76; // voltage at 25C
  const float average_slope = 0.0025; // 2.5mV/C
  return (((adc1_samples[1] * ADC_VOLTAGE_FACTOR) - V_25) / average_slope) + 25;
}

float adc_throttle_percentage(void) {
  // 0.85 - 2.55V when measured
  // say it's 0.87 to 2.53
  // range = 1.66V
  float voltage = adc2_samples[1] * ADC_VOLTAGE_FACTOR;
  float percentage = (voltage - 0.87)/1.66*100;
  if (percentage < 0) {
    percentage = 0;
  } else if (percentage > 100.0) {
    percentage = 100;
  }
  return percentage;
}

float adc_vref(void) {
  return adc1_samples[0] * ADC_VOLTAGE_FACTOR;
}

void adc_get_phase_voltages(float *buf) {
  chSysLock();
  buf[0] = buffered_current_sense_voltages[0] * ADC_VOLTAGE_FACTOR;
  buf[1] = buffered_current_sense_voltages[1] * ADC_VOLTAGE_FACTOR;
  chSysUnlock();
}
