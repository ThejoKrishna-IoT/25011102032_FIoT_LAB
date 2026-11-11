#include "main.h"

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    GPIO_PinState previous = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    uint32_t pressed_at = 0;
    uint32_t last_sample = 0;
    uint8_t pressed = 0;
    uint8_t long_action = 0;
    uint8_t blink_mode = 0;
    uint32_t last_blink = 0;

    while (1)
    {
        uint32_t now = HAL_GetTick();
        GPIO_PinState current = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

        if ((uint32_t)(now - last_sample) >= 20U)
        {
            last_sample = now;

            if (current == GPIO_PIN_RESET && previous == GPIO_PIN_SET && !pressed)
            {
                pressed = 1;
                pressed_at = now;
                long_action = 0;
            }

            if (current == GPIO_PIN_SET && previous == GPIO_PIN_RESET && pressed)
            {
                uint32_t duration = now - pressed_at;
                if (!long_action && duration < 1000U)
                {
                    if (!blink_mode)
                        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
                    else
                        blink_mode = 0;
                }
                pressed = 0;
            }

            previous = current;
        }

        if (pressed && !long_action && (uint32_t)(now - pressed_at) >= 1000U)
        {
            long_action = 1;
            blink_mode = !blink_mode;
            if (blink_mode)
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        }

        if (blink_mode && (uint32_t)(now - last_blink) >= 500U)
        {
            last_blink = now;
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
        }
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
