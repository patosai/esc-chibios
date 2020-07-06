#ifndef _ADC_H_
#define _ADC_H_

void adc_start_current_measurement_conversion(void);
void adc_stop_current_measurement_conversion(void);

float adc_vref(void);
float adc_phase_a_current(void);
float adc_phase_b_current(void);
float adc_phase_c_current(void);

#endif