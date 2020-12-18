#ifndef _ADC_H_
#define _ADC_H_

// only phase B and C
#define ADC_MOTOR_PHASES_SAMPLED 2

void adc_start_continuous_conversion(void);
void adc_stop_continuous_conversion(void);

float adc_temp_celsius(void);
float adc_throttle_percentage(void);
float adc_vref(void);

// buffer must be of size ADC_MOTOR_PHASES_SAMPLED
void adc_get_phase_voltages(float*);

#endif