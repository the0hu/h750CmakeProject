#ifndef __MYFFT_H_
#define __MYFFT_H_

#include "stdio.h"
#include "stm32h7xx_hal.h"
#include <arm_math.h>

#define LENGTH_SAMPLES_FFT 1024 //复数浮点序列fft的采样点数

//输入 ， 输出 。 幅值
extern float32_t input_fft[LENGTH_SAMPLES_FFT * 2];
extern float32_t output_fft[LENGTH_SAMPLES_FFT /2];
extern float32_t Mag[LENGTH_SAMPLES_FFT];

void My_cfft_f32(void);
void My_rfft_f32(void);

#endif