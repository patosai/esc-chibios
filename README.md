ESC Firmware
=========

This is a brushless DC motor controller based on the STM32F407VG. It runs on ChibiOS.

### Operating conditions
- Battery voltage: 9V-60V
- Max (safe) continuous current: 52A
    - MOSFET limit: 239A
        - package limit: 239A
        - thermal limit: 52A
            - https://www.infineon.com/dgdl/Infineon-an-994-AN-v06_00-EN.pdf?fileId=5546d46265f064ff01667ab5829d4d44
            - low side MOSFETs have about 1/2 sq. in. of copper, high side has more
            - Infineon tests in PDF show ~23C/W max thermal resistance for 1 sq. in., 43.7C/W for modified minimum footprint for D2PAK
            - linearly estimating to 1/2 sq. in = 33.3C/W thermal resistance
            - max junction temp is 175C, 33.3C/W junction-to-ambient. assuming a 40C ambient, 135C difference = 4.06W max dissipation.
            - at Rds = 1.5mOhm on a bad day, P = I^2*R, so I = 52A
    - Sense resistor limit: 141A
        - two 0.001Ohm sense resistors in parallel, each dissipating 5W (a ton of power...)
        - P = I^2*R, R = 0.0005Ohms, P = 10W, I = 141.4A
    - PCB limit without insane heating: ~80A
        - used various calculators online to guesstimate this, allowing for only 20C rise in PCB temp over ambient and 1oz copper

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
