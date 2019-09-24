ESC Firmware
=========

This is a brushless DC motor controller based on the STM32F407VG. It runs on ChibiOS.

V1 anomalies
-----
- VREF on the DRV8353RS is not connected to the filtered 3.3V
- the STM32F4 pins assigned to the DRV8353RS PWM inputs (PC4, PC5, PA6, PA7, PA4, PA5) are not connected to the same timer, or any timer at all
  - to remedy this, the USART1 TX/RX pins on PA9/PA10 and the USB OTG DM pin on PA11 can be reused for channels 1, 2, and 3 from timer 1
  - only the high pins for the DRV8353RS need to be rewired, since it should be programmed via SPI to operate in 3x PWM mode, which means the low pins are always 1, and the high pins control the half bridge output
  - PA9 <-> PC5, PA10 <-> PA5, PA11 <-> PA7 so channel 1 = phase A, channel 2 = phase B, channel 3 = phase C