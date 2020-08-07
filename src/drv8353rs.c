#include "drv8353rs.h"
#include "spi.h"

static const uint16_t bottom_11_bit_mask = (((uint16_t)1 << 11) - 1); // generates 0000011111111111
static const uint8_t bottom_4_bit_mask = (((uint8_t)1 << 4) - 1); // generates 00001111

static uint16_t read_spi2(drv8353rs_register_t addr) {
  // https://www.ti.com/lit/ds/symlink/drv8350.pdf
  // 8.5.1.1.1 SPI Format
  // bit 15 is R/W, bit 11-14 is address, bit 0-10 is data
  addr &= bottom_4_bit_mask;
  return spi2_exchange_word((1<<15) | (addr<<11)) & bottom_11_bit_mask;
}

static void write_spi2(drv8353rs_register_t addr, uint16_t data) {
  addr &= bottom_4_bit_mask;
  data &= bottom_11_bit_mask;
  spi2_exchange_word((0 << 15) | (addr << 11) | data);
}

static void enable_drv8353rs(void) {
  palSetPadMode(GPIOB, 11, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOB, 11);
  const uint8_t max_necessary_reset_pulse_time_us = 40;
  chThdSleepMicroseconds(max_necessary_reset_pulse_time_us);
  palSetPad(GPIOB, 11);
}

static void setup_nfault_pin(void) {
  palSetPadMode(GPIOB, 10, PAL_MODE_INPUT_PULLUP);
}

static void lock_drv_spi(void) {
  uint16_t command = read_spi2(GATE_DRIVE_HIGH_CONTROL);
  command &= ~(0b111 << 8);
  command |= 0b110 << 8;
  write_spi2(GATE_DRIVE_HIGH_CONTROL, command);
}

static void unlock_drv_spi(void) {
  uint16_t command = read_spi2(GATE_DRIVE_HIGH_CONTROL);
  command &= ~(0b111 << 8);
  command |= 0b011 << 8;
  write_spi2(GATE_DRIVE_HIGH_CONTROL, command);
}

void drv8353rs_init(void) {
  setup_nfault_pin();
  enable_drv8353rs();

  // https://www.st.com/content/ccc/resource/technical/document/reference_manual/3d/6d/5a/66/b4/99/40/d4/DM00031020.pdf/files/DM00031020.pdf/jcr:content/translations/en.DM00031020.pdf
  // section 28.5 SPI and I2S registers

  // DRV8353RS
  // http://www.ti.com/lit/ds/symlink/drv8353.pdf
  // section 8.5.1.1
  // The SCLK pin should be low when the nSCS pin transitions from high to low and from low to high.
  //   ->> CPOL = 1
  // Data is captured on the falling edge of SCLK and data is propagated on the rising edge of SCLK.
  //  -> data is propagated on the rising edge, so capture should happen on the falling edge
  //  -> since CPOL is 0, the falling edge is the second clock
  //  ->> CPHA = 1
  uint16_t cr1 =
      SPI_CR1_MSTR
      | SPI_CR1_CPHA // first data capture on second edge
      | SPI_CR1_DFF // 16-bit frame
      | SPI_CR1_BR_1 | SPI_CR1_BR_0 // SPI2 is on APB1, which is running at 21MHz according to mcuconf.h, set SPI clock to APB1/16 = 1.3125MHz
      ;
  uint16_t cr2 = 0;

  spi2_init(cr1, cr2);

  uint16_t tx_driver_control = 0 << 10 // associated half bridge shutdown in response to overcurrent
      | 0 << 9 // undervoltage lockout fault enabled
      | 1 << 7 // thermal warning reported on nFAULT and FAULT bit
      | 1 << 5 // 3x PWM mode  low input should be tied high, high input controls gate
      | 0 << 4 // 1x PWM mode uses synchronous rectification, doesn't apply since not using 1x PWM mode
      | 0 << 3 // in 1x PWM, this bit is ORed with INHC (DIR) input, doesn't apply since not using 1x PWM mode
      | 0 << 2 // don't coast the motor
      | 0 << 1 // don't brake the motor
      | 0 << 0 // don't clear latched fault bits
      ;
  write_spi2(DRIVER_CONTROL, tx_driver_control);

  // IRFS7530 has a gate-to-drain capacitance of 73nC
  // let's say a dog can hear a max frequency of 70kHz
  // PWM will run above 70kHz so the dog doesn't hear it as much
  // 1 period = 14.28us
  // 5% of time rising + 5% of time falling = 714ns
  // I = Q/t
  // minimum drive current = 73nC/714ns = 102mA
  // make it 150mA to be safe.
  uint16_t tx_gate_drive_high = 0b000 << 8 // don't lock settings just yet
      | 0b0011 << 4 // high side rise drive current = 150mA
      | 0b0011 << 0 // high side fall drive current = 300mA
      ;
  write_spi2(GATE_DRIVE_HIGH_CONTROL, tx_gate_drive_high);

  uint16_t tx_gate_drive_low = 0 << 10 // when overcurrent is set to automatic retrying fault, fault is cleared after tRETRY
      | 0b10 << 8 // gate current drive time should be ~714ns, allow check to be 2000ns
      | 0b0011 << 4 // low side rise drive current = 150mA
      | 0b0011 << 0 // low side fall drive current = 300mA
      ;
  write_spi2(GATE_DRIVE_LOW_CONTROL, tx_gate_drive_low);

  // sense resistor = 0.0005Ohm, 10W max dissipation
  // P = I^2*R -> I^2 = P/R = 20,000, so max I = 141.42A
  // voltage across the sense resistor is V = IR = 0.07V
  // set overcurrent voltage to 0.1V
  uint16_t tx_overcurrent_control = 0 << 10 // overcurrent time is 8ms
      | 0b01 << 8 // dead time (time between switching of high and low MOSFET, prevents shoot-through)
      | 0b01 << 6 // overcurrent causes an automatic retrying fault
      | 0b10 << 4 // overcurrent deglitch time (minimum time of overcurrent before detection) = 4us
      | 0b1101 << 0 // VDS overcurrent voltage = 0.1V
      ;
  write_spi2(OVERCURRENT_CONTROL, tx_overcurrent_control);

  uint16_t tx_current_sense_control = 0 << 10 // sense amplifier positive is SPx
      | 0 << 9 // sense amplifier reference voltage is VREF
      | 0 << 8 // overcurrent for low side MOSFET is measured from SHx to SPx
      | 0b10 << 6 // 20V/V shunt amplifier gain
      | 0 << 5 // sense overcurrent fault enabled
      | 0 << 4 // normal sense amplifier A operation
      | 0 << 3 // normal sense amplifier B operation
      | 0 << 2 // normal sense amplifier C operation
      | 0b11 << 0 // sense overcurrent voltage = 1V
      ;
  write_spi2(CURRENT_SENSE_CONTROL, tx_current_sense_control);

  uint16_t tx_driver_configuration = 1 << 0 // amplifier calibration uses internal auto calibration
      ;
  write_spi2(DRIVER_CONFIGURATION, tx_driver_configuration);

  lock_drv_spi();
}

void drv8353rs_manually_calibrate(void) {
  unlock_drv_spi();

  uint16_t current_sense = read_spi2(CURRENT_SENSE_CONTROL);
  current_sense |= 0b111 << 2; // turn on calibration for all 3 amplifiers
  write_spi2(CURRENT_SENSE_CONTROL, current_sense);

  chThdSleepMilliseconds(1000); // wait 1 second to calibrate

  current_sense &= ~(0b111 << 2); // return to normal operation
  write_spi2(CURRENT_SENSE_CONTROL, current_sense);

  lock_drv_spi();
}

bool drv8353rs_has_fault(void) {
  return palReadPad(GPIOB, 10) == PAL_LOW;
}

uint16_t drv8353rs_read_register(drv8353rs_register_t reg) {
  return read_spi2(reg);
}
