#include "main.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
static uint8_t rx;
static char input[512];
static volatile uint16_t input_len;
static volatile uint8_t line_ready;

static void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void send_uart(UART_HandleTypeDef *u, const char *s);
static void send_at(const char *s, uint32_t delay_ms);
static void serve_http(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    HAL_UART_Receive_IT(&huart3, &rx, 1);

    send_at("AT\r\n", 1000);
    send_at("AT+CWMODE=1\r\n", 1000);
    send_at("AT+CIPMUX=1\r\n", 1000);
    send_at("AT+CIPSERVER=1,80\r\n", 1000);
    send_uart(&huart2, "\r\nHTTP server configured on ESP8266 port 80.\r\n");

    while (1)
    {
        if (line_ready)
        {
            __disable_irq();
            line_ready = 0;
            input_len = 0;
            __enable_irq();
            serve_http();
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        if (input_len < sizeof(input) - 1U)
        {
            input[input_len++] = (char)rx;
            if (input_len >= 5U && strstr(input, "+IPD,") != NULL &&
                (strstr(input, "GET /") != NULL || strstr(input, "POST /") != NULL))
                line_ready = 1;
        }
        else
        {
            input_len = 0;
        }
        HAL_UART_Receive_IT(&huart3, &rx, 1);
    }
}

static void serve_http(void)
{
    char *p = strstr(input, "+IPD,");
    int link_id = 0;
    if (p == NULL) return;
    if (sscanf(p, "+IPD,%d,", &link_id) != 1) return;

    const char body[] = "<html><body><h1>STM32 IoT Node</h1><p>Hello from STM32</p></body></html>";
    char response[256];
    char command[40];
    int body_len = (int)strlen(body);
    int response_len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
        body_len, body);
    if (response_len <= 0 || response_len >= (int)sizeof(response)) return;

    snprintf(command, sizeof(command), "AT+CIPSEND=%d,%d\r\n", link_id, response_len);
    send_uart(&huart3, command);
    HAL_Delay(300);
    send_uart(&huart3, response);
    HAL_Delay(300);
    snprintf(command, sizeof(command), "AT+CIPCLOSE=%d\r\n", link_id);
    send_uart(&huart3, command);
    send_uart(&huart2, "Served HTTP GET request.\r\n");
}

static void send_at(const char *s, uint32_t delay_ms)
{
    send_uart(&huart3, s);
    HAL_Delay(delay_ms);
}

static void send_uart(UART_HandleTypeDef *u, const char *s)
{
    HAL_UART_Transmit(u, (uint8_t *)s, strlen(s), 3000);
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
