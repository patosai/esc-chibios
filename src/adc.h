#ifndef _ADC_H_
#define _ADC_H_

void adc_start_continuous_conversion(void);
void adc_stop_continuous_conversion(void);

float adc_temp(void);
float adc_vref(void);
void adc_retrieve_phase_currents(float*);

#endif