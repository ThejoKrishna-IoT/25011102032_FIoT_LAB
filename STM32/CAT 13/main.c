#include "main.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

#define BME280_ADDR (0x76U << 1)

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static HAL_StatusTypeDef bme_read(uint8_t reg, uint8_t *data, uint16_t len);
static HAL_StatusTypeDef bme_write(uint8_t reg, uint8_t value);
static void send_modem(const char *s);
static void modem_command(const char *s, uint32_t delay_ms);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();

    uint8_t chip_id = 0;
    if (bme_read(0xD0, &chip_id, 1) != HAL_OK || chip_id != 0x60U) Error_Handler();

    bme_write(0xF2, 0x01);
    bme_write(0xF4, 0x27);
    bme_write(0xF5, 0xA0);

    modem_command("AT\r\n", 500);
    modem_command("AT+CWMODE=1\r\n", 500);
    modem_command("AT+CWJAP=\"YOUR_SSID\",\"YOUR_PASSWORD\"\r\n", 8000);
    modem_command("AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n", 5000);

    send_modem("MQTT publish implementation must be linked here.\r\n");

    while (1)
    {
        uint8_t raw[8];
        if (bme_read(0xF7, raw, 8) == HAL_OK)
        {
            int32_t adc_p = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | (raw[2] >> 4);
            int32_t adc_t = ((int32_t)raw[3] << 12) | ((int32_t)raw[4] << 4) | (raw[5] >> 4);
            int32_t adc_h = ((int32_t)raw[6] << 8) | raw[7];
            char payload[96];
            snprintf(payload, sizeof(payload), "{\"raw_temperature\":%ld,\"raw_pressure\":%ld,\"raw_humidity\":%ld}",
                     (long)adc_t, (long)adc_p, (long)adc_h);
            send_modem(payload);
            send_modem("\r\n");
        }
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

static void send_modem(const char *s)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)s, strlen(s), 3000);
}

static void modem_command(const char *s, uint32_t delay_ms)
{
    send_modem(s);
    HAL_Delay(delay_ms);
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
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
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
