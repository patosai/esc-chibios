#ifndef _ADC_H_
#define _ADC_H_

void adc_init(void);
void adc_stop(void);
void adc_start_current_measurement_conversion(void);
void adc_stop_current_measurement_conversion(void);

float adc_vref(void);
float adc_phase_a_voltage(void);
float adc_phase_b_voltage(void);
float adc_phase_c_voltage(void);

#endif