#include <ch.h>
#include <hal.h>
#include <stdarg.h>

#include "adc.h"
#include "led.h"
#include "log.h"
#include "serial.h"

// On the STM32F407, all ADCs are run off of APB2
// Last time I checked mcuconf.h, APB2 runs at 42MHz

// samples_saved must be 1 or an even number
#define ADC_SAMPLES_SAVED_PER_CHANNEL 1
#define ADC1_CHANNELS 2
#define ADC2_CHANNELS 2
#define ADC3_CHANNELS 1

#define ADC_VOLTAGE_FACTOR (3.3 / 4095.0)
#define PHASE_RESISTANCE_OHMS 0.005
#define SAMPLE_TO_CURRENT_CONSTANT (ADC_VOLTAGE_FACTOR/PHASE_RESISTANCE_OHMS)

// TIM2 runs on APB1 which runs at 21MHz
#define GPT2_TIMER_FREQUENCY 21000 // 21kHz timer, it seems 10kHz works but not 1Khz, perhaps timer upper limit? TIM2 runs on APB1

static adcsample_t adc1_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC1_CHANNELS];
static adcsample_t adc2_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC2_CHANNELS];
static adcsample_t adc3_samples[ADC_SAMPLES_SAVED_PER_CHANNEL * ADC3_CHANNELS];

static void adc_callback(ADCDriver *adc) {
  (void)adc;
  led_4_toggle();
}

static void adc_common_error_callback(ADCDriver *adc, adcerror_t err) {
  (void)adc;
  (void)err;
  int adc_num = 0;
  if (adc->adc == ADC1) {
    adc_num = 1;
  } else if (adc->adc == ADC2) {
    adc_num = 2;
  } else if (adc->adc == ADC3)  {
    adc_num = 3;
  }
  log_println_in_interrupt("ADC %d error %d", adc_num, err);
}

static const ADCConversionGroup adc1_config = {
  .circular = FALSE,
  .num_channels = ADC1_CHANNELS,
  .end_cb = adc_callback,
  .error_cb = adc_common_error_callback,
  .cr1 = ADC_CR1_SCAN, // set scan mode so ADC will convert SQR1/2/3 channels
  // https://www.st.com/content/ccc/resource/technical/document/reference_manual/3d/6d/5a/66/b4/99/40/d4/DM00031020.pdf/files/DM00031020.pdf/jcr:content/translations/en.DM00031020.pdf
  // see section 13.6 Conversion on external trigger and trigger polarity
  .cr2 = ADC_CR2_ADON // turn ADC on
         | ADC_CR2_DMA // use DMA to transfer data to buffers
         | ADC_CR2_EXTEN_0 // trigger on rising edge
         | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1, // trigger on TIM2_TRGO
  .smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_480) | ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480), // 42MHz, 10us minimum sampling time, 480 ticks gives 11us
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = ADC_SQR2_SQ8_N(ADC_CHANNEL_SENSOR),
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};

static const ADCConversionGroup adc2_config = {
  .circular = FALSE,
  .num_channels = ADC2_CHANNELS,
  .end_cb = NULL,
  .error_cb = adc_common_error_callback,
  .cr1 = ADC_CR1_SCAN,
  .cr2 = ADC_CR2_ADON
         | ADC_CR2_DMA
         | ADC_CR2_EXTEN_0, // trigger from ADC1, requires ADC_CCR MULTI[4:0] to be set
  .smpr1 = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_480) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_480),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = ADC_SQR2_SQ8_N(ADC_CHANNEL_VREFINT),
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12)
};

static const ADCConversionGroup adc3_config = {
  .circular = FALSE,
  .num_channels = ADC3_CHANNELS,
  .end_cb = NULL,
  .error_cb = adc_common_error_callback,
  .cr1 = ADC_CR1_SCAN,
  .cr2 = ADC_CR2_ADON
         | ADC_CR2_DMA
         | ADC_CR2_EXTEN_0, // trigger from ADC1, requires ADC_CCR MULTI[4:0] to be set
  .smpr1 = ADC_SMPR1_SMP_AN13(ADC_SAMPLE_480),
  .smpr2 = 0,
  .sqr1 = 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN13)
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

  adcSTM32EnableTSVREFE();
  // STM32F4 uses the ChibiOS ADCv2 driver, which does not support dual/triple mode ADCs
  // Instead, write the ADC_CCR MULTI register manually
  // 10110 is triple mode, regular simultaneous mode conversion
  ADC->CCR = (ADC->CCR & ~0b11111) | 0b10110;

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

static void gpt_callback(GPTDriver *gpt) {
  (void)gpt;
  led_5_toggle();
}

static const GPTConfig gpt2_config = {
  .frequency = GPT2_TIMER_FREQUENCY,
  .callback = gpt_callback,
  // https://www.st.com/content/ccc/resource/technical/document/application_note/group0/91/01/84/3f/7c/67/41/3f/DM00236305/files/DM00236305.pdf/jcr:content/translations/en.DM00236305.pdf
  // see end of section 2.5
  // MMS[2:0] = 010 -> pulse on TRGO
  .cr2 = TIM_CR2_MMS_1,
  .dier = 0U
};

static void start_timer(void) {
  gptStart(&GPTD2, &gpt2_config);
  gptStartContinuous(&GPTD2, GPT2_TIMER_FREQUENCY / 1);
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

float adc_temp(void) {
  // formula from reference manual section 13.10 subsection "Reading the temperature"
  // constant values from F407VG datasheet
  const float V_25 = 0.76; // voltage at 25C
  const float average_slope = 0.0025; // 2.5mV/C
  return (((adc1_samples[1] * ADC_VOLTAGE_FACTOR) - V_25) / average_slope) + 25;
}

float adc_vref(void) {
  return adc2_samples[1] * ADC_VOLTAGE_FACTOR;
}

void adc_retrieve_phase_currents(float* buf) {
  // disable interrupts to prevent DMA from updating samples in the middle of retrieval
  chSysLock();
  buf[0] = adc1_samples[0];
  buf[1] = adc2_samples[0];
  buf[2] = adc3_samples[0];
  chSysUnlock();

  buf[0] = buf[0] * SAMPLE_TO_CURRENT_CONSTANT;
  buf[1] = buf[1] * SAMPLE_TO_CURRENT_CONSTANT;
  buf[2] = buf[2] * SAMPLE_TO_CURRENT_CONSTANT;
}
