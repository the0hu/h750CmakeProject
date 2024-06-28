#ifndef __TEST_H__
#define __TEST_H__

#include <main.h>
#include "arm_math.h"
#include <stdint.h>

#define MAGLENGTH   512
#define SAMP_FREQ   100000

typedef struct{
    float32_t mag[MAGLENGTH];
    float32_t wave_a, wave_b;//峰值
    uint16_t   index_a, index_b;//峰值的位置
    float32_t  freq_a, freq_b;
    uint8_t     type_a,type_b;//判断波形，规定0为正弦波，1为三角波;
}FftMagStruct;

extern void  Test(FftMagStruct *data);
void jude_wave(FftMagStruct *data);
void Slidingwindow_filter(uint16_t *input,uint16_t size,uint16_t windowsize,uint16_t *output);
extern void my_print(const char *ch );
void sendDataViaUSART(const uint32_t *data, int dataSize);
#endif
