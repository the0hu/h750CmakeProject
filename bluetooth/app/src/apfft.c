#include "apfft.h"

#define apFFTPasteLen(len) arm_cfft_sR_f32_len##len
#define apFFTCFFTInstance apFFTPasteLen(2048)

static float32_t normalizedConvWindowTable[apFFTSize * 2 - 1] = {};

static void calcNormalizedConvWindow(float32_t *pDst) {
    // 1. construct a N point hanning window
    float32_t hanning[apFFTSize] = {};
    const float32_t a0 = 0.5;

    for (uint32_t i = 0; i < apFFTSize; ++i) {
        float32_t theta = 2.0f * PI * i / (apFFTSize - 1);
        hanning[i] = a0 - (1.0f - a0) * arm_cos_f32(theta);
    }

    // 2. convolve the hanning window with itself
    float32_t convWindow[apFFTSize * 2 - 1];
    arm_conv_f32(hanning, apFFTSize, hanning, apFFTSize, convWindow);

    // 3. get sum of the convolved window
    float32_t sum = 0.0f;
    for (uint32_t i = 0; i < apFFTSize * 2 - 1; ++i) {
        sum += convWindow[i];
    }

    // 4. normalize the convolved window
    for (uint32_t i = 0; i < apFFTSize * 2 - 1; ++i) {
        pDst[i] = convWindow[i] / sum;
    }
}

/**
 * @brief Full-phase preprocessing for apFFT.
 * 对2N-1点原数据进行全相位预处理，得到N点数据。
 *
 * @param pSrc 2N-1点原数据(时域数据), 结果保存在pSrc中
 */
static void fullPhssePreprocessing(float32_t *pSrc) {
    // 1. mutilply the normalized conv window with the original data
    for (uint32_t i = 1; i < apFFTSize * 2 - 1; ++i) {
        pSrc[i] *= normalizedConvWindowTable[i];
    }

    // 2. add 0:N-1 point to 2N-1:N point
    for (uint32_t i = apFFTSize - 1; i > 0; --i) {
        pSrc[i] += pSrc[2 * apFFTSize - 1 - (apFFTSize - i)];
    }
    pSrc[0] = pSrc[apFFTSize] + 0;

    // 3. shift the data
    for (int32_t i = apFFTSize - 1; i >= 0; --i) {
        pSrc[i * 2] = pSrc[i];
        pSrc[i * 2 + 1] = 0.0f;
    }
}

extern void apfft_init(void) {
    calcNormalizedConvWindow(normalizedConvWindowTable);
}

extern void apfft(float32_t *pSrc) {
    fullPhssePreprocessing(pSrc);
    arm_cfft_f32(&apFFTCFFTInstance, pSrc, 0, 1);
}
