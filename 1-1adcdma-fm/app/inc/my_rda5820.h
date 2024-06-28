#ifndef __MY_RDA5820_H_
#define __MY_RDA5820_H_

#include "stdio.h"
#include "stm32h7xx_hal.h"


#define RDA_5820_ID     0X00
#define RDA_5820_ENABLE 0X02
#define RDA_5820_READ   0X23
#define RDA_5820_WIRTE  0X22
#define RDA_5820_MODE   0X40     
#define RDA_5820_TUNE   0X03



void RDA5820_Init(void);
void RDA5820_WR_Reg(uint8_t reg_addr, uint16_t val);
uint16_t RDA5820_RD_Reg(uint8_t reg_addr);

void RDA5820_RxMode(void);//接收模式
void RDA5820_TxMode(void);//发送模式
void RDA5820_SetBand(uint8_t band);
void RDA5820_SetSpace(uint8_t space);
void RDA5820_SetVol(uint8_t vol);
void RDA5820_SetFreq(uint16_t freq);
uint16_t RDA5820_GetFreq(void);

#endif