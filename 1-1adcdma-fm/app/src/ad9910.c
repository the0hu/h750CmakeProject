/**
 * @file ad9910.c
 * @author nmpassthf (nmpassthf@gmail.com),IpinZhu (zhuyiping742@gmail.com)
 * @brief
 * @date 2023-05-10
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */

#include "spi.h"
#include <ad9910.h>

#define AD9910_SPI &hspi1

#define littleDelay() msDelay(1)

// // 40M输入  25倍频  VC0=101   ICP=001;
// uint8_t cfrData[3][4] = {{0x00, 0x40, 0x00, 0x00},
//                          {0x01, 0x00, 0x00, 0x00},
//                          {0x05, 0x0f, 0x41, 0x32}};
const sAD9910_CFR
    ad9910DefDisableALLCFR = {.workmode = eAD9910_DISABLE,
                              .CFR1 = {eAD9910_PLAYBACK_AMP, true, 0},
                              .CFR3 = {0, 5, 1, true, true, 25}},
    ad9910DefRamModeCFR = {.workmode = eAD9910_RAM_MODE,
                           .CFR1 = {eAD9910_PLAYBACK_AMP, true, 0},
                           .CFR3 = {0, 5, 1, true, true, 25}},
    ad9910DefSingleModeCFR = {.workmode = eAD9910_SINGLE_TONE_MODE,
                              .CFR1 = {0, true, 0},
                              .CFR3 = {0, 5, 1, true, true, 25}},
    ad9910DefDRGModeCFR = {.workmode = eAD9910_DRGONLY_MODE,
                           .CFR1 = {0, true, 0},
                           .CFR3 = {0, 5, 1, true, true, 25}},
    ad9910DefDRGRamModeCFR = {.workmode = eAD9910_RAMDRG_MODE,
                              .CFR1 = {eAD9910_PLAYBACK_AMP, true, 0},
                              .CFR3 = {0, 5, 1, true, true, 25}};

static sAD9910_CFR _ad9910CurrentCFR;

static void __AD9910_writeCFR(const uint8_t cfrData[3][4]) {
    uint8_t cfrAddr = 0x00U;
    uint8_t i = 0;

    while (cfrAddr < 0x03U) {
        AD9910_CS_ENABLE();

        writeSPI(AD9910_SPI, &cfrAddr, 1);
        writeSPI(AD9910_SPI, cfrData[i], 4);

        AD9910_CS_DISABLE();

        i++;
        cfrAddr++;
    }

    AD9910_FLASH_DATA();
}

extern void AD9910_init() {
    writeGPIO(_PWR_GPIO_Port, _PWR_Pin, 0);
    AD9910_prfSelect(0);

    writeGPIO(_DRHOLD_GPIO_Port, _DRHOLD_Pin, 0);
    writeGPIO(_DRCTL_GPIO_Port, _DRCTL_Pin, 0);

    writeGPIO(_MASTER_RESET_GPIO_Port, _MASTER_RESET_Pin, 1);
    msDelay(10);
    writeGPIO(_MASTER_RESET_GPIO_Port, _MASTER_RESET_Pin, 0);
}

extern void AD9910_setCFR(sAD9910_CFR cfrData) {
    union {
        uint8_t cfr8b[3][4];
        uint32_t cfr32b[3];
    } cfrRawData = {0};

    _ad9910CurrentCFR = cfrData;

    cfrRawData.cfr8b[0][0] |= (uint8_t)cfrData.CFR1.ramPlaybackMode << 5U;
    cfrRawData.cfr8b[0][1] |= (uint8_t)cfrData.CFR1.inverseSincFilterEnable
                              << 6U;
    cfrRawData.cfr8b[0][1] |= (uint8_t)cfrData.CFR1.ramInternalProfileCtrl4b
                              << 1U;

    cfrRawData.cfr32b[0] |= (uint32_t)(0x02000000U);

    cfrRawData.cfr32b[1] |= (uint32_t)(0x00000000U);

    cfrRawData.cfr8b[2][0] |= (uint8_t)cfrData.CFR3.DRV0 << 4U;
    cfrRawData.cfr8b[2][0] |= (uint8_t)cfrData.CFR3.VCO_SEL << 0U;
    cfrRawData.cfr8b[2][1] |= (uint8_t)cfrData.CFR3.icpCurrent << 3U;
    cfrRawData.cfr8b[2][2] |= (uint8_t)cfrData.CFR3.REFCLKInputDivRestB << 6U;
    cfrRawData.cfr8b[2][2] |= (uint8_t)cfrData.CFR3.PLLEnable << 0U;
    cfrRawData.cfr8b[2][3] |= (uint8_t)cfrData.CFR3.N << 1U;

    cfrRawData.cfr32b[2] |= (uint32_t)(0x00000000U);

    switch (cfrData.workmode) {
        case eAD9910_DISABLE:
            break;
        case eAD9910_RAM_MODE:
            cfrRawData.cfr8b[0][0] |= (uint8_t)BIT(7);
            break;
        case eAD9910_SINGLE_TONE_MODE:
            cfrRawData.cfr8b[1][0] |= (uint8_t)BIT(0);
            break;
        case eAD9910_DRGONLY_MODE:
            cfrRawData.cfr8b[1][1] |= (uint8_t)(BIT(3) | BIT(2) | BIT(1));
            break;
        case eAD9910_RAMDRG_MODE:
            cfrRawData.cfr8b[0][0] |= (uint8_t)BIT(7);
            cfrRawData.cfr8b[1][1] |= (uint8_t)(BIT(3) | BIT(2) | BIT(1));
            break;

        default:
            break;
    }

    __AD9910_writeCFR(cfrRawData.cfr8b);
}

extern sAD9910_CFR AD9910_getCurrentCFR() { return _ad9910CurrentCFR; }

/**
 * @brief 发送single tone profile控制字
 *
 * @param profile
 */
static void __AD9910_sendSingleToneProfile(sSINGLE_TONE_PROFILE_REG profile,
                                           uint8_t profileNumber) {
    const uint8_t aimRegAddr = 0x0E + profileNumber;

    if (profileNumber > 7)
        return;

    uint8_t profileRawData[8] = {0};

    profile.amp14b *= 25.206153846153846153846153846154;
    profile.amp14b = profile.amp14b > 0x3FFF ? 0x3FFF : profile.amp14b;
    profile.freqTuning32b *= 4.294967296;

    profile.phaseOffset16b = profile.phaseOffset16b * 0xffff / 360;

    profileRawData[0] |= (uint8_t)(profile.amp14b >> 8U & 0x3FU);
    profileRawData[1] |= (uint8_t)(profile.amp14b & 0xFFU);
    profileRawData[2] |= (uint8_t)(profile.phaseOffset16b >> 8U & 0xFFU);
    profileRawData[3] |= (uint8_t)(profile.phaseOffset16b & 0xFFU);
    profileRawData[4] |= (uint8_t)(profile.freqTuning32b >> 24U & 0xFFU);
    profileRawData[5] |= (uint8_t)(profile.freqTuning32b >> 16U & 0xFFU);
    profileRawData[6] |= (uint8_t)(profile.freqTuning32b >> 8U & 0xFFU);
    profileRawData[7] |= (uint8_t)(profile.freqTuning32b & 0xFFU);

    AD9910_CS_ENABLE();

    writeSPI(AD9910_SPI, &aimRegAddr, 1);
    writeSPI(AD9910_SPI, profileRawData, 8);

    AD9910_CS_DISABLE();

    // TODO
}

/**
 * @brief 发送ram profile控制字
 *
 * @param profile
 */
static void __AD9910_sendRamProfile(sRAM_PROFILE_REG profile,
                                    uint8_t profileNumber) {
    uint8_t aimRegAddr = 0x0E + profileNumber;

    if (profileNumber > 7)
        return;

    uint8_t profileRawData[8] = {0};

    profileRawData[0] |= 0U;
    profileRawData[1] |= (uint8_t)(profile.step16b >> 8U & 0xFFU);
    profileRawData[2] |= (uint8_t)(profile.step16b & 0xFFU);
    profileRawData[3] |= (uint8_t)(profile.waveEndAddr10b >> 2U & 0xFFU);
    profileRawData[4] |= (uint8_t)((profile.waveEndAddr10b & 0x03U) << 6U);
    profileRawData[5] |= (uint8_t)(profile.waveBeginAddr10b >> 2U & 0xFFU);
    profileRawData[6] |= (uint8_t)((profile.waveBeginAddr10b & 0x03U) << 6U);
    profileRawData[7] |= (uint8_t)profile.nodwellHigh << 5U |
                         (uint8_t)profile.zeroCrossing << 3U |
                         (uint8_t)profile.ramWorkMode;

    AD9910_CS_ENABLE();

    writeSPI(AD9910_SPI, &aimRegAddr, 1);
    writeSPI(AD9910_SPI, profileRawData, 8);

    AD9910_CS_DISABLE();

    AD9910_FLASH_DATA();
}

/**
 * @brief 发送ramprofile数据
 *
 * @param data
 */
static void __AD9910_sendRAMDatas(const uint8_t data[]) {
    uint8_t pdat = 0x16U;

    AD9910_CS_ENABLE();
    writeSPI(AD9910_SPI, &pdat, 1);
    writeSPI(AD9910_SPI, data, 256);
    AD9910_CS_DISABLE();

    AD9910_FLASH_DATA();
}

/**
 * @brief 发送DRG数据
 *
 * @param data
 */
static void __AD9910_transmitDRG(const uint8_t data[]) {
    uint8_t pdat = 0x0b;
    AD9910_CS_ENABLE();

    writeSPI(AD9910_SPI, &pdat, 1);
    writeSPI(AD9910_SPI, data, 8);
    ++pdat;

    writeSPI(AD9910_SPI, &pdat, 1);
    writeSPI(AD9910_SPI, data + 8, 8);
    ++pdat;

    writeSPI(AD9910_SPI, &pdat, 1);
    writeSPI(AD9910_SPI, data + 16, 4);

    AD9910_CS_DISABLE();

    AD9910_FLASH_DATA();
}

/**
 * @brief 单一正弦波
 *
 * @param range 幅度输入范围：1-650 mV
 * @param freq 频率输入范围：1-400 000 000Hz
 *
 */
__weak void SineWave(uint16_t range, uint16_t phase, uint32_t freq) {
    AD9910_setCFR(ad9910DefSingleModeCFR);

    sSINGLE_TONE_PROFILE_REG prf = {range, phase, freq};
    __AD9910_sendSingleToneProfile(prf, 0);
}

/**
 * @brief 正弦扫频
 *
 * @param range 幅度
 * @param SweepMinFre 最小频率
 * @param SweepMaxFre 最大频率
 * @param SweepStepFre 步进频率
 * @param SweepTime 时间
 * @return
 */
__weak void SineSweep(uint16_t range, uint32_t SweepMinFre,
                      uint32_t SweepMaxFre, uint32_t SweepStepFre,
                      uint32_t SweepTime) {
    uint8_t para[20] = {};
    uint32_t Tmp_Min, Tmp_Max, Tmp_Step, Tmp_Time;

    sSINGLE_TONE_PROFILE_REG prf = {range, 0, 0};
    __AD9910_sendSingleToneProfile(prf, 0);

    Tmp_Min = (uint32_t)SweepMinFre * 4.294967296;
    if (SweepMaxFre > 400000000)
        SweepMaxFre = 400000000;
    Tmp_Max = (uint32_t)SweepMaxFre * 4.294967296;
    if (SweepStepFre > 400000000)
        SweepStepFre = 400000000;
    Tmp_Step = (uint32_t)SweepStepFre * 4.294967296;
    Tmp_Time = (uint32_t)SweepTime / 4;  // 1GHz/4, 单位：ns
    if (Tmp_Time > 0xffff)
        Tmp_Time = 0xffff;
    // 扫频上下限
    para[7] = (uint8_t)Tmp_Min;
    para[6] = (uint8_t)(Tmp_Min >> 8);
    para[5] = (uint8_t)(Tmp_Min >> 16);
    para[4] = (uint8_t)(Tmp_Min >> 24);
    para[3] = (uint8_t)Tmp_Max;
    para[2] = (uint8_t)(Tmp_Max >> 8);
    para[1] = (uint8_t)(Tmp_Max >> 16);
    para[0] = (uint8_t)(Tmp_Max >> 24);
    // 频率步进（单位：Hz）
    para[15] = (uint8_t)Tmp_Step;
    para[14] = (uint8_t)(Tmp_Step >> 8);
    para[13] = (uint8_t)(Tmp_Step >> 16);
    para[12] = (uint8_t)(Tmp_Step >> 24);
    para[11] = (uint8_t)Tmp_Step;
    para[10] = (uint8_t)(Tmp_Step >> 8);
    para[9] = (uint8_t)(Tmp_Step >> 16);
    para[8] = (uint8_t)(Tmp_Step >> 24);
    // 步进时间间隔（单位：us）
    para[19] = (uint8_t)Tmp_Time;
    para[18] = (uint8_t)(Tmp_Time >> 8);
    para[17] = (uint8_t)Tmp_Time;
    para[16] = (uint8_t)(Tmp_Time >> 8);

    __AD9910_transmitDRG(para);  // 写DRG

    AD9910_setCFR(ad9910DefDRGModeCFR);
}

/**
 * @brief 方波 64*32bit数据
 *
 * @param Sample_interval 频率
 * @return
 */
__weak void Square_wave(uint32_t Sample_interval) {
    const uint8_t ramdata_Square[] = {
        // 方波
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc,
        0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc,
        0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc,
        0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc,
        0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc,
        0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc,
        0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,
        0xFFU, 0xfc, 0x00,  0x00, 0xFFU, 0xfc, 0x00,  0x00,

        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00,
        0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00};
    AD9910_init();

    SineWave(650, 0, 0);

    sRAM_PROFILE_REG prf = {
        0,
        63,
        ((1000000000 / (unsigned long int)(Sample_interval) / 64 / 4)),
        false,
        false,
        eAD9910_CONTINUOUS_RECIRCULATE};
    __AD9910_sendRamProfile(prf, 0);

    __AD9910_sendRAMDatas(ramdata_Square);
    // 发送cfr控制字
    AD9910_setCFR(ad9910DefRamModeCFR);
}

/**
 * @brief 锯齿波 64*32bit数据
 *
 * @param Sample_interval 频率
 * @return
 */
__weak void Sawtooth_wave(uint32_t Sample_interval) {
    const uint8_t ramdata_Sawtooth[] = {
        // 锯齿波
        0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00,
        0x0b, 0xf4, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x13, 0xec, 0x00, 0x00,
        0x17, 0xe8, 0x00, 0x00, 0x1b, 0xe4, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00,
        0x23, 0xdc, 0x00, 0x00, 0x27, 0xd8, 0x00, 0x00, 0x2b, 0xd4, 0x00, 0x00,
        0x2f, 0xd0, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x37, 0xc8, 0x00, 0x00,
        0x3b, 0xc4, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x43, 0xbc, 0x00, 0x00,
        0x47, 0xb8, 0x00, 0x00, 0x4b, 0xb4, 0x00, 0x00, 0x4f, 0xb0, 0x00, 0x00,
        0x53, 0xac, 0x00, 0x00, 0x57, 0xa8, 0x00, 0x00, 0x5b, 0xa4, 0x00, 0x00,
        0x5f, 0xa0, 0x00, 0x00, 0x63, 0x9c, 0x00, 0x00, 0x67, 0x98, 0x00, 0x00,
        0x6b, 0x94, 0x00, 0x00, 0x6f, 0x90, 0x00, 0x00, 0x73, 0x8c, 0x00, 0x00,
        0x77, 0x88, 0x00, 0x00, 0x7b, 0x84, 0x00, 0x00,

        0x7f, 0x80, 0x00, 0x00, 0x83, 0x7c, 0x00, 0x00, 0x87, 0x78, 0x00, 0x00,
        0x8b, 0x74, 0x00, 0x00, 0x8f, 0x70, 0x00, 0x00, 0x93, 0x6c, 0x00, 0x00,
        0x97, 0x68, 0x00, 0x00, 0x9b, 0x64, 0x00, 0x00, 0x9f, 0x60, 0x00, 0x00,
        0xa3, 0x5c, 0x00, 0x00, 0xa7, 0x58, 0x00, 0x00, 0xab, 0x54, 0x00, 0x00,
        0xaf, 0x50, 0x00, 0x00, 0xb3, 0x4c, 0x00, 0x00, 0xb7, 0x48, 0x00, 0x00,
        0xbb, 0x44, 0x00, 0x00, 0xbf, 0x40, 0x00, 0x00, 0xc3, 0x3c, 0x00, 0x00,
        0xc7, 0x38, 0x00, 0x00, 0xcb, 0x34, 0x00, 0x00, 0xcf, 0x30, 0x00, 0x00,
        0xd3, 0x2c, 0x00, 0x00, 0xd7, 0x28, 0x00, 0x00, 0xdb, 0x24, 0x00, 0x00,
        0xdf, 0x20, 0x00, 0x00, 0xe3, 0x1c, 0x00, 0x00, 0xe7, 0x18, 0x00, 0x00,
        0xeb, 0x14, 0x00, 0x00, 0xef, 0x10, 0x00, 0x00, 0xf3, 0x0c, 0x00, 0x00,
        0xf7, 0x08, 0x00, 0x00, 0xfb, 0x04, 0x00, 0x00,

    };
    AD9910_init();

    SineWave(650, 0, 0);

    sRAM_PROFILE_REG prf = {
        0, 63,
        // 64,
        // 127,
        ((1000000000 / (unsigned long int)(Sample_interval) / 64 / 4)), false,
        false, eAD9910_CONTINUOUS_RECIRCULATE};
    __AD9910_sendRamProfile(prf, 0);
    __AD9910_sendRAMDatas(ramdata_Sawtooth);

    // 发送cfr
    AD9910_setCFR(ad9910DefRamModeCFR);
}

/**
 * @brief 三角波 64数据
 *
 * @param Sample_interval 频率
 * @return
 */
__weak void Triangluar_wave(uint32_t Sample_interval) {
    const uint8_t ramdata_Sawtooth[] = {
        // 锯齿波
        0x00, 0x00, 0x00, 0x00, 0x08, 0x44, 0x00, 0x00, 0x10, 0x44, 0x00, 0x00,
        0x18, 0xc8, 0x00, 0x00, 0x21, 0x08, 0x00, 0x00, 0x29, 0x4c, 0x00, 0x00,
        0x31, 0x8c, 0x00, 0x00, 0x39, 0xd0, 0x00, 0x00, 0x42, 0x10, 0x00, 0x00,
        0x4a, 0x54, 0x00, 0x00, 0x52, 0x94, 0x00, 0x00, 0x5a, 0xd8, 0x00, 0x00,
        0x63, 0x18, 0x00, 0x00, 0x6b, 0x5c, 0x00, 0x00, 0x73, 0x9c, 0x00, 0x00,
        0x7b, 0xe0, 0x00, 0x00, 0x84, 0x20, 0x00, 0x00, 0x8c, 0x64, 0x00, 0x00,
        0x94, 0xa4, 0x00, 0x00, 0x9c, 0xe8, 0x00, 0x00, 0xa5, 0x28, 0x00, 0x00,
        0xad, 0x6c, 0x00, 0x00, 0xb5, 0xac, 0x00, 0x00, 0xBD, 0xF0, 0x00, 0x00,
        0xC6, 0x30, 0x00, 0x00, 0xCE, 0x74, 0x00, 0x00, 0xD6, 0xB4, 0x00, 0x00,
        0xDE, 0xF8, 0x00, 0x00, 0xE7, 0x38, 0x00, 0x00, 0xEF, 0x7C, 0x00, 0x00,
        0xF7, 0xBC, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00,

        0xff, 0xfe, 0x00, 0x00, 0xF7, 0xBC, 0x00, 0x00, 0xEF, 0x7C, 0x00, 0x00,
        0xE7, 0x38, 0x00, 0x00, 0xDE, 0xF8, 0x00, 0x00, 0xD6, 0xB4, 0x00, 0x00,
        0xCE, 0x74, 0x00, 0x00, 0xC6, 0x30, 0x00, 0x00, 0xBD, 0xF0, 0x00, 0x00,
        0xb5, 0xac, 0x00, 0x00, 0xad, 0x6c, 0x00, 0x00, 0xa5, 0x28, 0x00, 0x00,
        0x9c, 0xe8, 0x00, 0x00, 0x94, 0xa4, 0x00, 0x00, 0x8c, 0x64, 0x00, 0x00,
        0x84, 0x20, 0x00, 0x00, 0x7b, 0xe0, 0x00, 0x00, 0x73, 0x9c, 0x00, 0x00,
        0x6b, 0x5c, 0x00, 0x00, 0x63, 0x18, 0x00, 0x00, 0x5a, 0xd8, 0x00, 0x00,
        0x52, 0x94, 0x00, 0x00, 0x4a, 0x54, 0x00, 0x00, 0x42, 0x10, 0x00, 0x00,
        0x39, 0xd0, 0x00, 0x00, 0x31, 0x8c, 0x00, 0x00, 0x29, 0x4c, 0x00, 0x00,
        0x21, 0x08, 0x00, 0x00, 0x18, 0xc8, 0x00, 0x00, 0x10, 0x44, 0x00, 0x00,
        0x08, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    };
    AD9910_init();

    SineWave(650, 0, 0);

    sRAM_PROFILE_REG prf = {
        0, 63,
        // 64,
        // 127,
        ((1000000000 / (unsigned long int)(Sample_interval) / 64 / 4)), false,
        false, eAD9910_CONTINUOUS_RECIRCULATE};
    __AD9910_sendRamProfile(prf, 0);
    __AD9910_sendRAMDatas(ramdata_Sawtooth);

    // 发送cfr
    AD9910_setCFR(ad9910DefRamModeCFR);
}
