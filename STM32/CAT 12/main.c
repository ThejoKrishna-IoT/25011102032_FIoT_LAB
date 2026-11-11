#include "main.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

#define BME280_ADDR (0x76U << 1)
#define ESP_UART huart3

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static HAL_StatusTypeDef bme_read(uint8_t reg, uint8_t *data, uint16_t len);
static HAL_StatusTypeDef bme_write(uint8_t reg, uint8_t value);
static void uart_send(UART_HandleTypeDef *u, const char *s);
static uint16_t dig_T1, dig_P1;
static int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static int32_t t_fine;

static int32_t compensate_temp(int32_t adc_T);
static uint32_t compensate_pressure(int32_t adc_P);
static uint32_t compensate_humidity(int32_t adc_H);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();

    uint8_t id = 0;
    if (bme_read(0xD0, &id, 1) != HAL_OK || id != 0x60U) Error_Handler();

    uint8_t c[24];
    bme_read(0x88, c, 24);
    dig_T1 = (uint16_t)(c[1] << 8 | c[0]);
    dig_T2 = (int16_t)(c[3] << 8 | c[2]);
    dig_T3 = (int16_t)(c[5] << 8 | c[4]);
    dig_P1 = (uint16_t)(c[7] << 8 | c[6]);
    dig_P2 = (int16_t)(c[9] << 8 | c[8]);
    dig_P3 = (int16_t)(c[11] << 8 | c[10]);
    dig_P4 = (int16_t)(c[13] << 8 | c[12]);
    dig_P5 = (int16_t)(c[15] << 8 | c[14]);
    dig_P6 = (int16_t)(c[17] << 8 | c[16]);
    dig_P7 = (int16_t)(c[19] << 8 | c[18]);
    dig_P8 = (int16_t)(c[21] << 8 | c[20]);
    dig_P9 = (int16_t)(c[23] << 8 | c[22]);

    uint8_t h[7];
    bme_read(0xA1, &h[0], 1);
    uint8_t h2[7];
    bme_read(0xE1, h2, 7);
    bme_write(0xF2, 0x01);
    bme_write(0xF4, 0x27);
    bme_write(0xF5, 0xA0);

    uart_send(&ESP_UART, "AT\r\n");
    HAL_Delay(500);
    uart_send(&ESP_UART, "AT+CWMODE=1\r\n");
    HAL_Delay(500);
    uart_send(&ESP_UART, "AT+CWJAP=\"YOUR_SSID\",\"YOUR_PASSWORD\"\r\n");
    HAL_Delay(8000);

    while (1)
    {
        uint8_t raw[8];
        bme_read(0xF7, raw, 8);
        int32_t adc_P = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | (raw[2] >> 4);
        int32_t adc_T = ((int32_t)raw[3] << 12) | ((int32_t)raw[4] << 4) | (raw[5] >> 4);
        int32_t adc_H = ((int32_t)raw[6] << 8) | raw[7];

        int32_t temp_x100 = compensate_temp(adc_T);
        uint32_t pressure = compensate_pressure(adc_P);
        uint32_t humidity = compensate_humidity(adc_H);

        char body[128], cmd[48], response[256];
        int body_len = snprintf(body, sizeof(body),
            "{\"temperature_c\":%ld.%02ld,\"pressure_pa\":%lu,\"humidity_x1024\":%lu}",
            (long)(temp_x100 / 100), (long)(temp_x100 >= 0 ? temp_x100 % 100 : -(temp_x100 % 100)),
            (unsigned long)pressure, (unsigned long)humidity);
        int response_len = snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
            body_len, body);

        uart_send(&huart2, body);
        uart_send(&huart2, "\r\n");
        uart_send(&ESP_UART, "AT+CIPMUX=1\r\n");
        HAL_Delay(200);
        uart_send(&ESP_UART, "AT+CIPSERVER=1,80\r\n");
        HAL_Delay(200);
        snprintf(cmd, sizeof(cmd), "AT+CIPSEND=0,%d\r\n", response_len);
        uart_send(&ESP_UART, cmd);
        HAL_Delay(300);
        uart_send(&ESP_UART, response);
        HAL_Delay(1000);
    }
}

static HAL_StatusTypeDef bme_read(uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

static HAL_StatusTypeDef bme_write(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c1, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
}

static int32_t compensate_temp(int32_t adc_T)
{
    int32_t v1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * dig_T2) >> 11;
    int32_t v2 = (((((adc_T >> 4) - (int32_t)dig_T1) * ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) * dig_T3) >> 14;
    t_fine = v1 + v2;
    return (t_fine * 5 + 128) >> 8;
}

static uint32_t compensate_pressure(int32_t adc_P)
{
    int64_t v1 = (int64_t)t_fine - 128000;
    int64_t v2 = v1 * v1 * dig_P6;
    v2 += (v1 * dig_P5) << 17;
    v2 += ((int64_t)dig_P4) << 35;
    v1 = ((v1 * v1 * dig_P3) >> 8) + ((v1 * dig_P2) << 12);
    v1 = (((((int64_t)1) << 47) + v1) * dig_P1) >> 33;
    if (v1 == 0) return 0;
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - v2) * 3125) / v1;
    v1 = ((int64_t)dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    v2 = ((int64_t)dig_P8 * p) >> 19;
    p = ((p + v1 + v2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)(p >> 8);
}

static uint32_t compensate_humidity(int32_t adc_H)
{
    int32_t v = t_fine - 76800;
    v = (((((adc_H << 14) - ((int32_t)0x8000)) -
        (((int32_t)h2_dummy) * v)) + 16384) >> 15);
    return (uint32_t)(v < 0 ? 0 : (v > 419430400 ? 419430400 : v));
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_USART3_UART_Init(void)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
