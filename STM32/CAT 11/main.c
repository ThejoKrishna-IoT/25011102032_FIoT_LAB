#include "main.h"

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    GPIO_PinState previous = GPIO_PIN_SET;
    uint32_t pressed_at = 0;
    uint32_t sample_at = 0;
    uint32_t blink_at = 0;
    uint8_t pressed = 0;
    uint8_t long_handled = 0;
    uint8_t blink_mode = 0;

    while (1)
    {
        uint32_t now = HAL_GetTick();
        GPIO_PinState current = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

        if ((uint32_t)(now - sample_at) >= 20U)
        {
            sample_at = now;
            if (current == GPIO_PIN_RESET && previous == GPIO_PIN_SET && !pressed)
            {
                pressed = 1;
                pressed_at = now;
                long_handled = 0;
            }
            if (current == GPIO_PIN_SET && previous == GPIO_PIN_RESET && pressed)
            {
                if (!long_handled && (uint32_t)(now - pressed_at) < 1000U)
                {
                    if (blink_mode) blink_mode = 0;
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
                }
                pressed = 0;
            }
            previous = current;
        }

        if (pressed && !long_handled && (uint32_t)(now - pressed_at) >= 1000U)
        {
            long_handled = 1;
            blink_mode = !blink_mode;
        }

        if (blink_mode && (uint32_t)(now - blink_at) >= 500U)
        {
            blink_at = now;
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef g = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    g.Pin = GPIO_PIN_5;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);
    g.Pin = GPIO_PIN_13;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &g);
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
