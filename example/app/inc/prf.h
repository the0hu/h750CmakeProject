/**
 * @file prf.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief pch in C language
 * @date 2023-07-27
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_PRF_H__
#define __M_PRF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "main.h"







/**
 * @brief 对HAL_GPIO_WritePin封装
 *
 */
#define writeGPIO(port, pin, value) \
    HAL_GPIO_WritePin(port, pin, (GPIO_PinState) !!(value))

/**
 * @brief 对HAL_SPI_Transmit封装
 *
 */
#define writeSPI(spiHandler, data, size) \
    HAL_SPI_Transmit(spiHandler, (uint8_t *)data, size, 20)

/**
 * @brief 对HAL_Delay封装
 *
 */
#define msDelay(x) HAL_Delay(x)

/**
 * @brief 第x位为1
 *
 */
#define BIT(x) (0x01ULL << x)

/**
 * @brief 获取多字节数据中的第x字节
 *
 */
#define XBYTE(x, data) ((data >> x * 8U) & 0xFFU)

/**
 * @brief 判断数据中低x位是否有效(无效高位是否为0)
 *
 */
#define IS_BIT_VALID(x, data) (!(data >> x))

#ifdef __cplusplus
}
#endif

#endif /* __M_PRF_H__ */
