#include "main.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
static uint8_t esp_rx;
static char esp_line[256];
static uint16_t esp_pos;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void uart_send(UART_HandleTypeDef *u, const char *s);
static void esp_command(const char *cmd, uint32_t wait_ms);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();

    HAL_UART_Receive_IT(&huart3, &esp_rx, 1);
    uart_send(&huart2, "\r\nCAT10 STM32-Wemos AT interface\r\n");

    esp_command("AT\r\n", 1000);
    esp_command("AT+GMR\r\n", 1000);
    esp_command("AT+CWMODE=1\r\n", 1000);
    esp_command("AT+CWLAP\r\n", 8000);

    uart_send(&huart2, "\r\nEnter Wi-Fi command as AT command in this terminal.\r\n");
    while (1) {}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        HAL_UART_Transmit(&huart2, &esp_rx, 1, 100);
        if (esp_pos < sizeof(esp_line) - 1U)
            esp_line[esp_pos++] = (char)esp_rx;
        if (esp_rx == '\n' || esp_pos >= sizeof(esp_line) - 1U)
        {
            esp_line[esp_pos] = '\0';
            esp_pos = 0;
        }
        HAL_UART_Receive_IT(&huart3, &esp_rx, 1);
    }
}

static void esp_command(const char *cmd, uint32_t wait_ms)
{
    uart_send(&huart2, "STM32 -> Wemos: ");
    uart_send(&huart2, cmd);
    uart_send(&huart3, cmd);
    HAL_Delay(wait_ms);
}

static void uart_send(UART_HandleTypeDef *u, const char *s)
{
    HAL_UART_Transmit(u, (uint8_t *)s, strlen(s), 2000);
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
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_8;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &g);
    g.Pin = GPIO_PIN_9;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &g);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
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
