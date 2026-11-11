# CAT8 — STM32 GPIO, switch debounce and press classification

Target: STM32F103C8T6 Blue Pill. Keil MDK 6 / Arm Compiler 6.
PA0 is the active-low push button (external pull-up or internal pull-up); PB12 is the LED output.
Short press (<1 s) toggles the LED. Long press (>=1 s) enters/exits continuous blink mode.
A 20 ms sampling interval provides debounce; duration is measured with HAL_GetTick without blocking.

Create the CubeMX project for STM32F103C8Tx, select the HAL driver, and add `main.c`/`main.h`.
Clock is HSI 8 MHz in this standalone source. Ensure the board LED is wired to PB12 as specified here;
Blue Pill onboard LED is commonly PC13 and is active-low, so change PB12 to PC13 if using that LED.
