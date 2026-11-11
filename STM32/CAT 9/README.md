# CAT9 — UART command controlled LED

Target: STM32F103C8T6 Blue Pill. USART1: PA9 TX, PA10 RX, 115200 8-N-1.
LED output: PB12 (change GPIO if your lab circuit uses another pin).
Commands terminated by Enter: `LED ON`, `LED OFF`, `LED TOGGLE`.
Use a 3.3 V USB-UART adapter and common GND. Do not connect 5 V UART logic to STM32 RX.
The receive interrupt accumulates one line and processes complete commands.
For CubeMX/MDK 6, select STM32F103C8Tx and add the STM32F1 HAL/CMSIS startup and system files.
