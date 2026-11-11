#ifndef ESPLIB_H
#define ESPLIB_H

#include "main.h"

#define WIFI_SSID "SNUC"
#define WIFI_PASS "snu12345"
#include <stdint.h>
#include <stdbool.h>

/* Wi-Fi Operating Modes */
typedef enum {
    StationMode = 1,
    SoftAPMode  = 2,
    CombinedMode = 3
} ESP8266_Mode_t;

/* MQTT Quality of Service (QoS) */
typedef enum {
    QoS0 = 0,
    QoS1 = 1,
    QoS2 = 2
} MQTT_QoS_t;

/* MQTT Connection Flags */
typedef enum {
    CleanSession = 0x02,
    WillFlag     = 0x04,
    WillQoS0     = 0x00,
    WillRetain   = 0x20,
    PasswordFlag = 0x40,
    UserFlag     = 0x80
} MQTT_ConnectFlag_t;

/* MQTT Client Configuration Structure */
typedef struct {
    char *clientID;
    char *username;
    char *password;
    uint16_t keepAlive;
    uint8_t connectFlag;
} MQTT_InitTypeDef_t;

/* Global Externs */
extern UART_HandleTypeDef *esp_huart;
extern uint8_t uartRxData;
extern char mqttTopic[64];
extern char mqttData[256];
extern volatile uint8_t mqtt_msg_received;

/* Function Prototypes per Syllabus Specification */
uint8_t ESP8266_Init(ESP8266_Mode_t mode, UART_HandleTypeDef huart);
uint8_t ESP8266_wifiConnect(const char *ssid, const char *pass);
uint8_t ESP8266_portConnect(const char *proto, const char *host, const char *port);

uint8_t MQTT_connectBroker(MQTT_InitTypeDef_t *setting);
uint8_t MQTT_publishTopic(const char *topic, const char *payload, MQTT_QoS_t qos);
uint8_t MQTT_subscribeTopic(const char *topic, uint8_t qos);
void MQTT_disconnect(void);

void MQTTTimer(void);
void MQTTDataHandler(char *topic, char *data);
void ESP8266_ReceiveHandler(uint8_t rx_byte);

#endif /* ESPLIB_H */
