#include "stm32h750xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include <my_rda5820.h>
#include <stdint.h>

uint16_t freq;

extern void Test() {
    RDA5820_SetBand(0);
    RDA5820_SetSpace(2);
    RDA5820_SetVol(15);
    RDA5820_RxMode();
    RDA5820_SetFreq(10000);

    HAL_Delay(100);
    freq = RDA5820_GetFreq();
    
}
