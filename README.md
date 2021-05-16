ESC Firmware
=========
[![CI badge](https://circleci.com/gh/patosai/esc-chibios.svg?style=svg)](https://app.circleci.com/pipelines/github/patosai/esc-chibios)

This is a brushless DC motor controller based on the STM32F407VG. It runs on ChibiOS.

### Compile for STM32F4 target
- `make`

### Flash to STM32F4 target
- `make flash` or `make f`

### Running cross-compiled tests
- `make test` or `make t`

### Debugging
- Launch the gdb server with `st-util` or `make gdb-server`
- Launch gdb using `gdb-multiarch`
    - `file build/esc.elf` to load symbols
    - `target extended-remote :4242`
    - or, do all the above with `make gdb`
- do debugging stuff

### Timers
- Timer 1 - used for PWM1
- Timer 2 - used to trigger ADCs
- Timer 4 - used to track motor rotor position
