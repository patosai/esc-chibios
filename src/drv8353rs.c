#include "adc.h"
#include "constants.h"
#include "drv8353rs.h"
#include "line.h"
#include "spi.h"


static const uint16_t bottom_11_bit_mask = 0b11111111111;
static const uint8_t bottom_4_bit_mask = 0b1111;

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
  chThdSleepMicroseconds(1); // ensure nSCS 400ns minimum high time
  spi2_exchange_word((0 << 15) | (addr << 11) | data);
}

static void enable_drv8353rs(void) {
  palSetLineMode(LINE_DRV8353RS_ENABLE, PAL_MODE_OUTPUT_PUSHPULL);
  palClearLine(LINE_DRV8353RS_ENABLE);
  const uint8_t max_necessary_reset_pulse_time_us = 40;
  chThdSleepMicroseconds(max_necessary_reset_pulse_time_us);
  palSetLine(LINE_DRV8353RS_ENABLE);
}

static void setup_nfault_pin(void) {
  palSetLineMode(LINE_DRV8353RS_NFAULT, PAL_MODE_INPUT_PULLUP);
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
      // no CPOL - idle clock is low
      | SPI_CR1_CPHA // data captured/propagated on second edge
      | SPI_CR1_DFF // 16-bit frame
      | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0 // SPI2 is on APB1, which is running at 10.5MHz according to mcuconf.h, set SPI clock to APB1/256 = 41kHz
      ;
  uint16_t cr2 = 0;

  spi2_init(cr1, cr2);

  uint16_t tx_driver_control = 0 << 10 // associated half bridge shutdown in response to overcurrent
      | 0 << 9 // undervoltage lockout fault enabled
      | 0 << 8 // gate drive fault enabled
      | 1 << 7 // thermal warning reported on nFAULT and FAULT bit
      | 1 << 5 // 3x PWM mode - low controls Hi-Z, high side controls on/off when low side pin is high
      | 0 << 4 // 1x PWM mode uses synchronous rectification, doesn't apply since not using 1x PWM mode
      | 0 << 3 // in 1x PWM, this bit is ORed with INHC (DIR) input, doesn't apply since not using 1x PWM mode
      | 0 << 2 // don't coast the motor
      | 0 << 1 // don't brake the motor
      | 0 << 0 // don't clear latched fault bits
      ;
  write_spi2(DRIVER_CONTROL, tx_driver_control);

  // IRFS7530 has total gate charge at VDS=48V and VGS=12V of ~285nC
  // let's say a dog can hear a max frequency of 50kHz
  // PWM will run at 50kHz so the dog doesn't hear it as much
  // APB2 runs at 84MHz and needs to be exact multiple of PWM freq, so make PWM freq 50kHz
  // 1 period = 20us
  // every half period = 10us
  // 5% of time rising/5% of time falling = 500ns
  // I = Q/t
  // minimum drive current = 285nC/500ns = 0.57A
  // closest greater option is 600mA
  // choose 950 since 600 is giving gate driver faults on the high side
  uint16_t tx_gate_drive_high = 0b000 << 8 // don't lock settings just yet
      | 0b1011 << 4 // high side rise drive current = 600mA
      | 0b0100 << 0 // high side fall drive current = 600mA
      ;
  write_spi2(GATE_DRIVE_HIGH_CONTROL, tx_gate_drive_high);

  uint16_t tx_gate_drive_low = 0 << 10 // when overcurrent is set to automatic retrying fault, fault is cleared after tRETRY
      | 0b10 << 8 // gate current drive time should be ~714ns, allow check to be 2000ns
      | 0b1010 << 4 // high side rise drive current = 600mA
      | 0b0100 << 0 // high side fall drive current = 600mA
      ;
  write_spi2(GATE_DRIVE_LOW_CONTROL, tx_gate_drive_low);

  // IRFS7530 max RDS = 2.4mOhm at 180C
  // maximum estimated I = 40A
  // voltage across the sense resistor is V = IR = 0.096V
  // set overcurrent voltage to 0.1V
  uint16_t tx_overcurrent_control = 0 << 10 // overcurrent time is 8ms
      | 0b01 << 8 // 100ns dead time (time between switching of high and low MOSFET, prevents shoot-through)
      | 0b01 << 6 // overcurrent causes an automatic retrying fault
      | 0b10 << 4 // overcurrent deglitch time (minimum time of overcurrent before detection) = 4us
      | 0b0100 << 0 // VDS overcurrent voltage = 0.1V
      ;
  write_spi2(OVERCURRENT_CONTROL, tx_overcurrent_control);

  // maximum estimated I = 40A
  // maximum common mode input range is +-0.15V, so max resistor size is 0.15V/40A = 3.75mOhm, power dissipation = I^2R = 6W..
  // targeting power dissipation = 0.5W, R = P/I^2 = 0.5/(40^2) = 0.3125mOhm, so make sense resistor 0.5mOhm
  // max SPx-SNx = IR = 40*0.0005 = 0.02V
  // SOx goes from 0.25 to Vref-0.25 = 3.3-0.25 = 3.05V, so 40V/V gain should be good
  // SOx goes from (Vref/2)-(40*0.02) to (Vref/2)+(40*0.02) = 0.85V to 2.45V
  uint8_t current_sense_amplification = 0b00;
  switch (DRV_CURRENT_SENSE_AMPLIFICATION) {
    case 5:
      current_sense_amplification = 0b00;
      break;
    case 10:
      current_sense_amplification = 0b01;
      break;
    case 20:
      current_sense_amplification = 0b10;
      break;
    case 40:
      current_sense_amplification = 0b11;
      break;
    default:
      chDbgAssert(false, "invalid current sense amplification amount");
      break;
  }
  uint16_t tx_current_sense_control = 0 << 10 // sense amplifier positive is SPx
      | 1 << 9 // sense amplifier reference voltage is VREF/2 (bidirectional current sense)
      | 0 << 8 // overcurrent for low side MOSFET is measured from SHx to SPx
      | (current_sense_amplification & 0b11) << 6
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

  adc_start_continuous_conversion();
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
  return palReadLine(LINE_DRV8353RS_NFAULT) == PAL_LOW;
}

uint16_t drv8353rs_read_register(drv8353rs_register_t reg) {
  return read_spi2(reg);
}