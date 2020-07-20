ESC Firmware
=========

This is a brushless DC motor controller based on the STM32F407VG. It runs on ChibiOS.

### Operating conditions
- Battery voltage: 9V-60V
- Max continuous current: 100A
    - MOSFET limit: 239A
    - Sense resistor limit: 141A
    - PCB limit without insane heating: ~100A

### Debugging
- Launch the gdb server with `st-util` or `make gdb-server`
- Launch gdb using `gdb-multiarch`
    - `file build/esc.elf` to load symbols
    - `target extended-remote :4242`
    - or, do all the above with `make gdb`
- do debugging stuff

#### V1 anomalies
STM32F4
- [x] the STM32F4 pins assigned to the DRV8353RS PWM inputs (PC4, PC5, PA6, PA7, PA4, PA5) are not connected to a timer
- [ ] JTAG NRST is only connected to the NJTRST pin on the STM32F4, and not NRESET
    - might not be a problem, but maybe add a solder bridge or a header

DRV8353R
- [x] VREF on the DRV8353RS is not connected to the filtered 3.3V
- [x] DRV8353RS SDO and SDI are backwards (SDO should be connected to SDI on MCU, and vice versa)
- [x] STM32F4/DRV8353RS SPI NSS and MISO pins should have an external pullup (test this), which may allow for use of alternate fn on B12 instead of manual NSS clearing/setting
    - pullup on NSS can be avoided by setting the MCU pin to be output pullup when SPI is not being used
- Add a larger cap (>10uF) to VM on DRV8353RS

Misc.
- [x] 2.2uF capacitor pad needs to be bigger
- [ ] 0.001 ohm sense resistor pad could be bigger to make soldering easier
- [ ] Add more space next to MOSFETs for heatsinks? Or maybe heatsinks can be added right on the packages themselves
- [ ] Add VDD on internal planes for more current capacity
