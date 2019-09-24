#include "motor.h"

#include "ch.h"
#include "hal.h"

void motor_pwm_init(void) {
	// due to v1 anomalies, high pins should be set to Hi-Z
	palSetPadMode(GPIOC, 4, PAL_MODE_OUTPUT_PUSHPULL); // phase A low
	palSetPadMode(GPIOC, 5, PAL_MODE_UNCONNECTED); // phase A high
	palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL); // phase B low
	palSetPadMode(GPIOA, 5, PAL_MODE_UNCONNECTED); // phase B high
	palSetPadMode(GPIOA, 6, PAL_MODE_OUTPUT_PUSHPULL); // phase C low
	palSetPadMode(GPIOA, 7, PAL_MODE_UNCONNECTED); // phase C high

	// rewire some other pins for TIM1 PWM
	// TODO should PAL_MODE_STM32_ALTERNATE_PUSHPULL be used?
	palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(1)); // TIM1 channel 1
	palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(1)); // TIM1 channel 2
	palSetPadMode(GPIOA, 11, PAL_MODE_ALTERNATE(1)); // TIM1 channel 3

	// low pins should be set to 1 due to 3x PWM mode
	palSetPad(GPIOC, 4);
	palSetPad(GPIOA, 4);
	palSetPad(GPIOA, 6);

	PWMConfig pwmConfig = {
			.frequency = 10000000, // 10MHz PWM clock frequency
			.period = 140, // ~71.428kHz frequency
			.callback = NULL,
			.channels = {
					{.mode = PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_DISABLED, .callback = NULL},
					{.mode = PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_DISABLED, .callback = NULL},
					{.mode = PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_DISABLED, .callback = NULL},
					{.mode = PWM_OUTPUT_DISABLED, .callback = NULL}
			}
	};

	pwmStart(&PWMD1, &pwmConfig);
}

void motor_set_speed(uint16_t rpm) {
	// TODO:w
	(void)rpm;
}
