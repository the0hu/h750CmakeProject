/**
 * @file ad9910.h
 * @author nmpassthf (nmpassthf@gmail.com),IpinZhu (zhuyiping742@gmail.com)
 * @brief
 * @date 2023-05-10
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */

#ifndef __M_AD9910_H__
#define __M_AD9910_H__

#include <prf.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct sRAM_PROFILE_REG {
    uint16_t waveBeginAddr10b;
    uint16_t waveEndAddr10b;
    uint16_t step16b;
    bool nodwellHigh;
    bool zeroCrossing;

    enum {
        eAD9910_DIRECT_SWITCH = 0U,                  // 直接转换模式
        eAD9910_RAMP_UP = 1U,                        // 上斜坡模式
        eAD9910_BIDIRECTION_RAMP = 2U,               // 双向斜坡模式
        eAD9910_CONTINUOUS_BIDIRECTIONAL_RAMP = 3U,  // 连续双向斜坡模式
        eAD9910_CONTINUOUS_RECIRCULATE = 4U          // 连续循环模式
    } ramWorkMode;
} sRAM_PROFILE_REG;

typedef struct sSINGLE_TONE_PROFILE_REG {
    uint16_t amp14b;
    uint16_t phaseOffset16b;
    uint32_t freqTuning32b;
} sSINGLE_TONE_PROFILE_REG;

typedef struct sAD9910_CFR {
    enum {
        eAD9910_DISABLE,
        eAD9910_RAM_MODE,
        eAD9910_SINGLE_TONE_MODE,
        eAD9910_DRGONLY_MODE,
        eAD9910_RAMDRG_MODE
    } workmode;

    struct {
        enum {
            eAD9910_PLAYBACK_FREQ = 0U,   // 频率
            eAD9910_PLAYBACK_PHASE = 1U,  // 相位
            eAD9910_PLAYBACK_AMP = 2U,    // 幅值
            eAD9910_PLAYBACK_POLAR = 3U,  // 极性
        } ramPlaybackMode;
        bool inverseSincFilterEnable;
        uint8_t ramInternalProfileCtrl4b;
    } CFR1;

    struct {
        uint8_t DRV0;
        uint8_t VCO_SEL;
        uint8_t icpCurrent;
        bool REFCLKInputDivRestB;
        bool PLLEnable;
        uint8_t N;
    } CFR3;

} sAD9910_CFR;

extern const sAD9910_CFR ad9910DefDisableALLCFR, ad9910DefRamModeCFR,
    ad9910DefSingleModeCFR, ad9910DefDRGModeCFR, ad9910DefDRGRamModeCFR;

#define AD9910_prfSelect(x)                               \
    do {                                                  \
        writeGPIO(_PFL0_GPIO_Port, _PFL0_Pin, x & 0x01U); \
        writeGPIO(_PFL1_GPIO_Port, _PFL1_Pin, x & 0x02U); \
        writeGPIO(_PFL2_GPIO_Port, _PFL2_Pin, x & 0x04U); \
    } while (0)

/**
 * @brief 初始化ad9910 默认ad9910DefDisableALLCFR中的配置初始化
 *
 */
extern void AD9910_init();

/**
 * @brief 设置CFR寄存器的值
 *
 *
 * @param cfrData
 */
extern void AD9910_setCFR(sAD9910_CFR cfrData);
extern sAD9910_CFR AD9910_getCurrentCFR();

extern void SineWave(uint16_t range,uint16_t phase, uint32_t Freq);
extern void Square_wave(uint32_t Sample_interval);
extern void Triangluar_wave(uint32_t Sample_interval);
extern void Sawtooth_wave(uint32_t Sample_interval);
extern void SineSweep(uint16_t range, uint32_t SweepMinFre,
                      uint32_t SweepMaxFre, uint32_t SweepStepFre,
                      uint32_t SweepTime);
                      
#define AD9910_CS_ENABLE() writeGPIO(_IO_RST_GPIO_Port, _IO_RST_Pin, 0)
#define AD9910_CS_DISABLE() writeGPIO(_IO_RST_GPIO_Port, _IO_RST_Pin, 1)

#define AD9910_UPD_ENABLE() writeGPIO(_IOU_GPIO_Port, _IOU_Pin, 1)
#define AD9910_UPD_DISABLE() writeGPIO(_IOU_GPIO_Port, _IOU_Pin, 0)

#define AD9910_FLASH_DATA()   \
    do {                      \
        AD9910_UPD_ENABLE();  \
        littleDelay();        \
        AD9910_UPD_DISABLE(); \
        littleDelay();        \
    } while (0)

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif /* __M_AD9910_H__ */
