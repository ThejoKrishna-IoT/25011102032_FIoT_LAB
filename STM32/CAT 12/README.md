# CAT12 — BME280 measurements over HTTP/JSON

Target: NUCLEO-F401RE + BME280 I2C + Wemos ESP8266 AT firmware.
I2C1: PB8 SCL, PB9 SDA; BME280 address 0x76 (SDO low; use 0x77 if SDO high).
USART3 PD8/PD9 is the Wemos AT link; USART2 is the terminal.
BME280 CSB must be high for I2C; power sensor at 3.3 V and ensure SDA/SCL pull-ups to 3.3 V.
This source demonstrates sensor ID check, calibration reads, measurement compensation, JSON formatting and modem UART transmission.

IMPORTANT: this package needs a full review against the exact BME280 module/library and your lab wiring before hardware use. The supplied lab sheet references Bosch's official `bme280.c` API; this self-contained draft does not substitute for validating the Bosch API path. Join credentials are placeholders. The HTTP server/client exchange must be completed with a proper +IPD/CIPSEND parser for a live Node-RED demonstration.
