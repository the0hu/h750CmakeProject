#include "stm32h7xx_hal.h"
#include "my_rda5820.h"
#include <lcd_spi_169.h>
#include <stdint.h>
#include <my_i2c.h>

//rda5820初始化,复位->读取芯片ID->上电
void RDA5820_Init(void){
    uint32_t id=0;
    id = RDA5820_RD_Reg(RDA_5820_ID);
    LCD_DisplayNumber(90, 20, (int32_t)id, 6);
    if (id == 0x5820) {
        
    }
    else {
    LCD_DisplayString(80, 50, "id error");
    }
    RDA5820_WR_Reg(RDA_5820_ENABLE,0x0002);    //软复位
    HAL_Delay(100);
    RDA5820_WR_Reg(RDA_5820_ENABLE,0xc001);
    HAL_Delay(800);
    
}


void RDA5820_WR_Reg(uint8_t reg_addr, uint16_t val){
    /*
    uint8_t tx_data[2];
    tx_data[0]=(uint8_t)(val>>8) & 0xff;
    tx_data[1]=(uint8_t)(val & 0xff);
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Write(&hi2c1, RDA_5820_WIRTE, reg_addr, I2C_MEMADD_SIZE_8BIT, tx_data, sizeof(tx_data), HAL_MAX_DELAY);
    if (status != HAL_OK) {
        LCD_DisplayString(80,80, "wr error");
        // 发生错误，可以在这里处理
    }*/
    I2C_Start();
    I2C_SendByte(RDA_5820_WIRTE);
    I2C_Get_ACK();
    I2C_SendByte(reg_addr);
    I2C_Get_ACK();
    I2C_SendByte(val >> 8);
    I2C_Get_ACK();
    I2C_SendByte(val & 0xff);
    I2C_Get_ACK();
    I2C_Stop();
    
}

uint16_t RDA5820_RD_Reg(uint8_t reg_addr){
    /*
    uint16_t data;
    uint8_t buff[2]={0,0};
    HAL_StatusTypeDef status;
    HAL_I2C_Master_Transmit(&hi2c1,RDA_5820_WIRTE,&reg_addr,1,HAL_MAX_DELAY);
    status = HAL_I2C_Master_Receive(&hi2c1,RDA_5820_READ,buff,2,HAL_MAX_DELAY);
    if (status != HAL_OK) {
        LCD_DisplayString(80,120, "rd error");
        // 发生错误，可以在这里处理
    }
    data=((uint16_t)buff[0] << 8) | buff[1];
    return data;
    */
    uint16_t data = 0;
    I2C_Start();
    I2C_SendByte(RDA_5820_WIRTE);
    I2C_Get_ACK();
    I2C_SendByte(reg_addr);
    I2C_Get_ACK();
    I2C_Start();
    I2C_SendByte(RDA_5820_READ);
    I2C_Get_ACK();
    data = I2C_ReadByte(1);
    data <<= 8;
    data |= I2C_ReadByte(0);
    I2C_Stop();
    return data;

    
}

//接收模式
void RDA5820_RxMode(void){
    uint16_t temp;
    temp = RDA5820_RD_Reg(RDA_5820_MODE);
    temp &=0xfff0;
    RDA5820_WR_Reg(RDA_5820_MODE,temp);
}

//发送模式
void RDA5820_TxMode(void){
    uint16_t temp;
    temp = RDA5820_RD_Reg(RDA_5820_MODE);
    temp &=0xfff0;
    temp |=0x0001;
    RDA5820_WR_Reg(RDA_5820_MODE,temp);
}

//设置频率范围
//band: 0,87-108Mhz; 1,76~91Mhz; 2,76-108Mhz; 3,65-76MHz or 50-65MHz
void RDA5820_SetBand(uint8_t band){
    uint16_t temp;
    temp = RDA5820_RD_Reg(RDA_5820_TUNE);
    temp &=0xfff3;
    temp |=band<<2;
    RDA5820_WR_Reg(RDA_5820_TUNE,temp);
}

//设置频率步进
//0,100Khz; 1,200Khz; 2,50Khz;
void RDA5820_SetSpace(uint8_t space){
    uint16_t temp;
    temp = RDA5820_RD_Reg(RDA_5820_TUNE);
    temp &=0xfffc;
    temp |=space;
    RDA5820_WR_Reg(RDA_5820_TUNE,temp);
}

//设置音量，vol:0~15;
void RDA5820_SetVol(uint8_t vol){
    uint16_t temp;
    temp = RDA5820_RD_Reg(0x05); //读取0X05的内容
    temp &= 0xfff0;
    temp |= vol & 0x0f;
    RDA5820_WR_Reg(0x05, temp);
}

//设置频率
void RDA5820_SetFreq(uint16_t freq){
    uint16_t temp;
    uint8_t band =0,space =0;
    uint16_t freq_botom,chan;

    temp = RDA5820_RD_Reg(RDA_5820_TUNE);
    temp &= 0x001f;
    band = (temp>>2) & 0x03;//获取频宽      Bit[3:2]位
    space = temp & 0x03;    //获取频率步进  Bit[1:0]位

    //获取频宽最小频率
    if(band == 0) freq_botom = 8700;
    else if(band == 1 || band == 2) freq_botom = 7600;
    else {
        freq_botom = RDA5820_RD_Reg(0x53);//
        freq_botom *= 10;
    }
    //获取频率步进
    if(space == 0) space = 10;
    else if(space == 1) space= 20;
    else  space = 5;

    if(freq < freq_botom) return;
    //公式  freq = freq_botom + chan*space;
    chan = (freq - freq_botom) / space;
    chan &= 0x3ff;
    temp |= chan<<6;
    temp |= 1<<4;
    RDA5820_WR_Reg(RDA_5820_TUNE,temp);
    HAL_Delay(50);
    while( (RDA5820_RD_Reg(0x0b) & (1<<7)) == 0) ;

}


//获取当前频率
//返回值:频率值(单位10Khz)
uint16_t RDA5820_GetFreq(void){
    uint16_t temp=0;
    uint8_t band=0,space=0;
    uint16_t freq_botom,chan;

    temp = RDA5820_RD_Reg(RDA_5820_TUNE);
    chan = temp >> 6;
    band = (temp>>2) & 0x03;//获取频宽      Bit[3:2]位
    space = temp & 0x03;    //获取频率步进  Bit[1:0]位

    //获取频宽最小频率
    if(band == 0) freq_botom = 8700;
    else if(band == 1 || band == 2) freq_botom = 7600;
    else {
        freq_botom = RDA5820_RD_Reg(0x53);
        freq_botom *= 10;
    }
    //获取频率步进
    if(space == 0) space = 10;
    else if(space == 1) space= 20;
    else  space = 5;

    temp = freq_botom + chan*space;
    return temp;
}