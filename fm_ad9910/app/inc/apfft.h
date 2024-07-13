/**
 * @file apfft.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief apfft in C language with arm_math library.
 * @date 2023-07-27
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_APFFT_H__
#define __M_APFFT_H__

#include "prf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <arm_const_structs.h>
#include <arm_math.h>

/**
 * @brief N点apFFT
 * 时域点数为2N-1，频域点数为N。
 * 修改N时须同时修改apFFTSize和apFFTCFFTInstance
 */
#define apFFTSize 1024

extern void apfft_init(void);

/**
 * @brief apFFT using float32 data type.
 * apFFT 对2N-1点时域数据进行FFT变换，得到N点频域数据。
 * 更改apFFT点数需要重新计算normalizedConvWindowTable
 *
 * @param pSrc 原始数据(时域数据), 结果保存在pSrc中，为Complex类型
 * pSrc[2N]，输入前2N-1为时域数据
 */
extern void apfft(float32_t *pSrc);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif /* __M_APFFT_H__ */
