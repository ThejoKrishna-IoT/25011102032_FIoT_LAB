# CAT13 — BME280 sensor and MQTT

Target: NUCLEO-F401RE + BME280 + Wemos D1 mini ESP8266.
I2C1 PB8/PB9, BME280 address 0x76 (SDO low; otherwise 0x77).
USART3 PD8/PD9 for Wemos; USART2 for terminal. Power logic at 3.3 V and common ground.
The assignment sheet specifies the instructor's ESP8266 custom MQTT API (`ESP8266_Init`, `ESP8266_wifiConnect`, `ESP8266_portConnect`, `MQTT_connectBroker`, `MQTT_publishTopic`) and topic `stm32/test/data`.
That custom library is not included in the uploaded assignment file. This project therefore contains the hardware bring-up and BME280 raw-register read path, but does NOT claim a completed MQTT packet implementation. Link the exact instructor-provided ESP8266/MQTT library and use its declared API before submission.
