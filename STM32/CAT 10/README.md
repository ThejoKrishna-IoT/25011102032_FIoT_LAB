# CAT10 — STM32 to Wemos ESP8266 AT UART

Target: NUCLEO-F401RE. USART2 PA2/PA3 is used for the PC terminal (board ST-LINK VCP routing must be checked against the Nucleo revision). USART3 PD8/PD9 is used for the Wemos AT link.
Wemos TX -> PD9 (STM32 RX); Wemos RX <- PD8 (STM32 TX); GND common. Both UART logic levels are 3.3 V.
The firmware tests AT, queries firmware, selects station mode and scans Wi-Fi networks. It forwards Wemos replies to the PC terminal.
For reliable operation, power the Wemos from a stable 5 V supply with adequate current. This CAT is the AT communication/command investigation exercise; Wi-Fi credentials are not hard-coded.
