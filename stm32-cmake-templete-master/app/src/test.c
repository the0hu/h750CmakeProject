#include "stm32h750xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"

extern void Test() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(1000);
}
