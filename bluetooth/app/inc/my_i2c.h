#ifndef __MY_I2C_H_
#define __MY_I2C_H_

#include "stm32h7xx_hal.h"
#include <gpio.h>

#define I2C_Address     0x54 //按实际
#define I2C_SCL_GPIOx   GPIOB
#define I2C_SCL_Pin     GPIO_PIN_10
#define I2C_SDA_GPIOx   GPIOB
#define I2C_SDA_Pin     GPIO_PIN_12


// IIC所有操作函数
void I2C_Delay(void);              // I2C延时函数
void I2C_Start(void);              //发送I2C开始信号
void I2C_Stop(void);                //发送I2C停止信号
void I2C_Send_ACK(uint8_t ack);
uint8_t I2C_Get_ACK(void);
uint8_t I2C_SendByte(uint8_t dat); // I2C发送一个字节
uint8_t I2C_ReadByte(uint8_t ack); // I2C读取一个字节
uint8_t I2C_Write_Reg(uint8_t reg, uint8_t data);
uint8_t I2C_Read_Reg(uint8_t reg);
uint8_t I2C_Read_Len(uint8_t reg, uint8_t len, uint8_t *buf);
uint8_t I2C_Write_Len(uint8_t reg, uint8_t len, uint8_t *buf);


#endif