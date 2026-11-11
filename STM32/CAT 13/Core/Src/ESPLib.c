/**
 ******************************************************************************
 * @file           : ESPLib.c
 * @brief          : Custom Built ESP8266 MQTT Client Library Implementation
 *                   for STM32F401RE NUCLEO and EMQX Cloud Broker
 ******************************************************************************
 */

#include "ESPLib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Static variables */
static UART_HandleTypeDef esp_uart_inst;
UART_HandleTypeDef *esp_huart = &esp_uart_inst;

uint8_t uartRxData = 0;
char mqttTopic[64] = {0};
char mqttData[256] = {0};
volatile uint8_t mqtt_msg_received = 0;

#define ESP_BUF_SIZE 512
static char esp_rx_buf[ESP_BUF_SIZE];
static volatile uint16_t esp_rx_head = 0;
static uint32_t last_ping_tick = 0;
static uint16_t current_keepalive = 500;
static volatile uint32_t mqtt_timer_tick = 0;

/**
 * @brief Send raw string command to ESP8266 and wait for expected response
 */
static uint8_t ESP_Send_Wait(const char *cmd, const char *expected, uint32_t timeout_ms)
{
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    esp_rx_head = 0;

    HAL_UART_Transmit(esp_huart, (uint8_t *)cmd, strlen(cmd), 1000);

    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms)
    {
        if (strstr(esp_rx_buf, expected) != NULL)
        {
            return 1; /* Success */
        }
        if (strstr(esp_rx_buf, "ERROR") != NULL || strstr(esp_rx_buf, "FAIL") != NULL)
        {
            return 0; /* Failure */
        }
    }
    return 0; /* Timeout */
}

/**
 * @brief Send raw byte buffer via AT+CIPSEND
 */
static uint8_t ESP_Send_Raw(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    char cipsend_cmd[32];
    sprintf(cipsend_cmd, "AT+CIPSEND=%u\r\n", len);

    if (!ESP_Send_Wait(cipsend_cmd, ">", 2000))
    {
        return 0;
    }

    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    esp_rx_head = 0;

    HAL_UART_Transmit(esp_huart, (uint8_t *)data, len, timeout_ms);

    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms)
    {
        if (strstr(esp_rx_buf, "SEND OK") != NULL)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Initialize ESP8266 module on specified UART
 */
uint8_t ESP8266_Init(ESP8266_Mode_t mode, UART_HandleTypeDef huart)
{
    esp_uart_inst = huart;

    /* Flush receive buffer and verify modem presence */
    ESP_Send_Wait("AT\r\n", "OK", 1000);
    ESP_Send_Wait("ATE0\r\n", "OK", 1000); /* Disable echo */

    /* Configure Wi-Fi mode */
    char mode_cmd[32];
    sprintf(mode_cmd, "AT+CWMODE=%d\r\n", (int)mode);
    if (!ESP_Send_Wait(mode_cmd, "OK", 1000))
        return 0;

    /* Configure Single Connection mode */
    ESP_Send_Wait("AT+CIPMUX=0\r\n", "OK", 1000);

    return 1;
}

/**
 * @brief Connect ESP8266 to local 802.11 Wi-Fi Access Point
 */
uint8_t ESP8266_wifiConnect(const char *ssid, const char *pass)
{
    char cmd[128];
    const char *target_ssid = (ssid != NULL && strlen(ssid) > 0) ? ssid : "SNUC";
    const char *target_pass = (pass != NULL && strlen(pass) > 0) ? pass : "snu12345";
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", target_ssid, target_pass);
    return ESP_Send_Wait(cmd, "WIFI GOT IP", 15000);
}

/**
 * @brief Establish TCP connection to remote MQTT broker (e.g. broker.emqx.io:1883)
 */
uint8_t ESP8266_portConnect(const char *proto, const char *host, const char *port)
{
    char cmd[128];
    sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%s\r\n", proto, host, port);
    return ESP_Send_Wait(cmd, "CONNECT", 10000);
}

/**
 * @brief Build and transmit MQTT CONNECT packet
 */
uint8_t MQTT_connectBroker(MQTT_InitTypeDef_t *setting)
{
    uint8_t packet[128];
    uint16_t idx = 0;
    uint16_t client_id_len = strlen(setting->clientID);
    uint16_t rem_len = 10 + (2 + client_id_len);

    current_keepalive = setting->keepAlive;

    /* Fixed Header */
    packet[idx++] = 0x10; /* CONNECT Packet Type */
    packet[idx++] = (uint8_t)rem_len;

    /* Variable Header: Protocol Name ("MQTT") */
    packet[idx++] = 0x00;
    packet[idx++] = 0x04;
    packet[idx++] = 'M';
    packet[idx++] = 'Q';
    packet[idx++] = 'T';
    packet[idx++] = 'T';

    /* Protocol Level: 4 (MQTT v3.1.1) */
    packet[idx++] = 0x04;

    /* Connect Flags */
    packet[idx++] = setting->connectFlag;

    /* Keep Alive (Seconds in Big-Endian) */
    packet[idx++] = (uint8_t)(setting->keepAlive >> 8);
    packet[idx++] = (uint8_t)(setting->keepAlive & 0xFF);

    /* Payload: Client Identifier */
    packet[idx++] = (uint8_t)(client_id_len >> 8);
    packet[idx++] = (uint8_t)(client_id_len & 0xFF);
    memcpy(&packet[idx], setting->clientID, client_id_len);
    idx += client_id_len;

    return ESP_Send_Raw(packet, idx, 3000);
}

/**
 * @brief Build and transmit MQTT PUBLISH packet
 */
uint8_t MQTT_publishTopic(const char *topic, const char *payload, MQTT_QoS_t qos)
{
    uint8_t packet[384];
    uint16_t idx = 0;
    uint16_t topic_len = strlen(topic);
    uint16_t payload_len = strlen(payload);
    uint16_t rem_len = 2 + topic_len + payload_len;

    /* Fixed Header: 0x30 | (qos << 1) */
    packet[idx++] = 0x30 | ((uint8_t)qos << 1);
    
    /* Remaining Length */
    if (rem_len < 128)
    {
        packet[idx++] = (uint8_t)rem_len;
    }
    else
    {
        packet[idx++] = (uint8_t)((rem_len & 0x7F) | 0x80);
        packet[idx++] = (uint8_t)(rem_len >> 7);
    }

    /* Variable Header: Topic Name */
    packet[idx++] = (uint8_t)(topic_len >> 8);
    packet[idx++] = (uint8_t)(topic_len & 0xFF);
    memcpy(&packet[idx], topic, topic_len);
    idx += topic_len;

    /* Payload */
    memcpy(&packet[idx], payload, payload_len);
    idx += payload_len;

    return ESP_Send_Raw(packet, idx, 3000);
}

/**
 * @brief Build and transmit MQTT SUBSCRIBE packet
 */
uint8_t MQTT_subscribeTopic(const char *topic, uint8_t qos)
{
    uint8_t packet[128];
    uint16_t idx = 0;
    uint16_t topic_len = strlen(topic);
    uint16_t packet_id = 0x0001;
    uint16_t rem_len = 2 + 2 + topic_len + 1;

    /* Fixed Header: 0x82 (SUBSCRIBE) */
    packet[idx++] = 0x82;
    packet[idx++] = (uint8_t)rem_len;

    /* Variable Header: Packet Identifier */
    packet[idx++] = (uint8_t)(packet_id >> 8);
    packet[idx++] = (uint8_t)(packet_id & 0xFF);

    /* Payload: Topic & Requested QoS */
    packet[idx++] = (uint8_t)(topic_len >> 8);
    packet[idx++] = (uint8_t)(topic_len & 0xFF);
    memcpy(&packet[idx], topic, topic_len);
    idx += topic_len;
    packet[idx++] = qos;

    return ESP_Send_Raw(packet, idx, 3000);
}

/**
 * @brief SysTick timer hook for MQTT keepAlive ping
 */
void MQTTTimer(void)
{
    mqtt_timer_tick++;
}

/**
 * @brief MQTT Application Callback when data is received from broker
 */
void MQTTDataHandler(char *topic, char *data)
{
    if (mqtt_msg_received)
    {
        /* Process received MQTT payload */
        mqtt_msg_received = 0;
    }
}

/**
 * @brief Interrupt service routine helper to accumulate UART bytes
 */
void ESP8266_ReceiveHandler(uint8_t rx_byte)
{
    if (esp_rx_head < (ESP_BUF_SIZE - 1))
    {
        esp_rx_buf[esp_rx_head++] = (char)rx_byte;
        esp_rx_buf[esp_rx_head] = '\0';
    }
    else
    {
        esp_rx_head = 0;
    }
}
