#ifndef _ADC_H_
#define _ADC_H_

#include <stdbool.h>

extern bool adc_has_new_sample_for_motor;

// only phase B and C
#define ADC_MOTOR_PHASES_SAMPLED 2

void adc_start_continuous_conversion(void);
void adc_stop_continuous_conversion(void);

float adc_temp_celsius(void);
float adc_throttle_percentage(void);
float adc_vref(void);

// phase A is not measured
float adc_phase_b_voltage(void);
float adc_phase_c_voltage(void);

#endif