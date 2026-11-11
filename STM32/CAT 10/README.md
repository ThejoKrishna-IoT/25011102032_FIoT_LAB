# CAT10C — ESP8266 HTTP server controlled by STM32

Target: NUCLEO-F401RE + Wemos D1 mini running ESP8266 AT firmware.
USART3 PD8 TX / PD9 RX is used for the modem. USART2 is the PC terminal.
The modem is configured with AT, AT+CWMODE=1, AT+CIPMUX=1 and AT+CIPSERVER=1,80.
A browser/Node-RED HTTP Request node connects to `http://<ESP8266_IP>/`.
The firmware parses the `+IPD,<link>,<length>:` prefix, sends a 200 response with computed Content-Length using the two-stage CIPSEND sequence, then closes the link.

Important: This lab-grade parser is intended for a short GET request. For robust production use, replace the fixed receive buffer with a streaming parser/ring buffer, parse the full +IPD payload length, wait for the `>` CIPSEND prompt and `SEND OK`, and handle disconnect/reconnect events. Wi-Fi join command/credentials are intentionally left to the lab setup.
