#include "drv8353rs.h"
#include "spi.h"

#define ADDR_FAULT_STATUS_1 0x00
#define ADDR_FAULT_STATUS_2 0x01
#define ADDR_DRIVER_CONTROL 0x02
#define ADDR_GATE_DRIVE_HIGH_CONTROL 0x03
#define ADDR_GATE_DRIVE_LOW_CONTROL 0x04
#define ADDR_OVERCURRENT_CONTROL 0x05
#define ADDR_CURRENT_SENSE_CONTROL 0x06
#define ADDR_DRIVER_CONFIGURATION 0x07

static uint16_t read(uint8_t addr) {
	addr &= 0b1111;
	return spi2_exchange_synchronous(1, 1 << 15 | addr << 11);
}

static void write(uint8_t addr, uint16_t data) {
	addr &= 0b1111;
	data &= 0b0000011111111111;
	spi2_exchange_synchronous(1, 0 << 15 | addr << 11 | data);
}

void drv8353rs_init(void) {
	// http://www.ti.com/lit/ds/symlink/drv8353.pdf
	// section 8.6.2 Control Registers
	uint16_t tx_driver_control = 0 << 10 // associated half bridge shutdown in response to overcurrent
			| 0 << 9 // undervoltage lockout fault enabled
			| 1 << 7 // thermal warning reported on nFAULT and FAULT bit
			| 0b01 << 5 // 3x PWM mode - low input should be tied high, high input controls gate
			| 0 << 4 // 1x PWM mode uses synchronous rectification, doesn't apply since not using 1x PWM mode
			| 0 << 3 // in 1x PWM, this bit is ORed with INHC (DIR) input, doesn't apply since not using 1x PWM mode
			| 0 << 2 // don't coast the motor
			| 0 << 1 // don't brake the motor
			| 0 << 0 // don't clear latched fault bits
			;
	write(ADDR_DRIVER_CONTROL, tx_driver_control);

	// IRFS7530 has a gate-to-drain capacitance of 73nC
	// let's say a dog can hear a max frequency of 70kHz
	// PWM will run above 70kHz so the dog doesn't hear it as much
	// 1 period = 14.28us
	// 1% of time rising/falling = 142ns
	// I = Q/t
	// minimum drive current = 73nC/142ns = 514mA
	// make it 700mA to be safe.
	uint16_t tx_gate_drive_high = 0b000 << 8 // don't lock settings just yet
			| 0b1011 << 4 // high side rise drive current = 700mA
			| 0b0101 << 0 // high side fall drive current = 700mA
			;
	write(ADDR_GATE_DRIVE_HIGH_CONTROL, tx_gate_drive_high);

	uint16_t tx_gate_drive_low = 0 << 10 // when overcurrent is set to automatic retrying fault, fault is cleared after tRETRY
			| 0b01 << 8 // 1000-ns peak gate current drive time
			| 0b1011 << 4 // low side rise drive current = 700mA
			| 0b0101 << 0 // low side fall drive current = 700mA
			;
	write(ADDR_GATE_DRIVE_LOW_CONTROL, tx_gate_drive_low);

	// this senses the voltage difference across the MOSFET drain and source
	// IRFS7530 rDS_ON(max) = 1.4mOhm
	// max current is about 150A
	// VDS overcurrent voltage ~ rDS_ON(max) * max_current = 1.4mOhm * 150A = 0.21V
	uint16_t tx_overcurrent_control = 0 << 10 // overcurrent time is 8ms
			| 0b01 << 8 // dead time (time between switching of high and low MOSFET, prevents shoot-through)
			| 0b01 << 6 // overcurrent causes an automatic retrying fault
			| 0b10 << 4 // overcurrent deglitch time (minimum time of overcurrent before detection) = 4us
			| 0b0101 << 0 // VDS overcurrent voltage = 0.2V
			;
	write(ADDR_OVERCURRENT_CONTROL, tx_overcurrent_control);

	uint16_t tx_current_sense_control = 0 << 10 // sense amplifier positive is SPx
			| 0 << 9 // sense amplifier reference voltage is VREF
			| 0 << 8 // overcurrent for low side MOSFET is measured from SHx to SPx
			| 0b01 << 6 // 10V/V shunt amplifier gain
			| 0 << 5 // sense overcurrent fault enabled
			| 0 << 4 // normal sense amplifier A operation
			| 0 << 3 // normal sense amplifier B operation
			| 0 << 2 // normal sense amplifier C operation
			| 0b11 << 0 // sense overcurrent voltage = 1V
			;
	write(ADDR_CURRENT_SENSE_CONTROL, tx_current_sense_control);

	uint16_t tx_driver_configuration = 1 << 0 // amplifier calibration uses internal auto calibration
			;
	write(ADDR_DRIVER_CONFIGURATION, tx_driver_configuration);
}

void drv8353rs_manually_calibrate(void) {
	uint16_t current_sense = read(ADDR_CURRENT_SENSE_CONTROL);
	current_sense |= 0b111 << 2; // turn on calibration for all 3 amplifiers
	write(ADDR_CURRENT_SENSE_CONTROL, current_sense);

	chThdSleepMilliseconds(1000); // wait 1 second to calibrate

	current_sense &= ~(0b111 << 2); // return to normal operation
	write(ADDR_CURRENT_SENSE_CONTROL, current_sense);
}

bool drv8353rs_has_fault(void) {
	uint16_t fault_1 = read(ADDR_FAULT_STATUS_1);
	return fault_1 & (1 << 10);
}
