#include "test.h"
#include "lcd_spi_169.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <usart.h>
#include <stdint.h>



extern void my_print(const char *ch ){
    HAL_UART_Transmit(&huart2, (uint8_t *)ch, strlen(ch), 0xff);
  
}

/**
 * @brief 串口打印
 *
 * @param 
 */
void sendDataViaUSART(const uint32_t *data, int dataSize) {
    int BUFFER_SIZE = dataSize * 6;
    char buffer[BUFFER_SIZE]; // 用于存储转换后的字符数据
    int index = 0;

    char *ptr = buffer;
    for (index = 0; index < dataSize; index++) {
        int len = snprintf(ptr, BUFFER_SIZE - (ptr - buffer), "%d\n", data[index]);
        if (len >= BUFFER_SIZE - (ptr - buffer)) {
            // 缓冲区不够大
            // 处理错误或者重新分配更大的缓冲区
            break;
        }
        ptr += len; // 将 ptr 指针移动到下一个可用位置
    }

    my_print(buffer);
}


/**
 * @brief 将数组的值在lcd上用曲线画出
 *
 * @param 
 */
void DrawCurveOnLCD(uint32_t a[1024]){
    uint16_t x; // 绘制点的 x 坐标
    uint16_t y; // 绘制点的 y 坐标
    uint32_t color; // 点的颜色

    // 找出数组中的最大值和最小值，用于归一化
    uint32_t max_value = 0;
    uint32_t min_value = UINT32_MAX; // 设置为最大可能的值

    for (uint16_t i = 0; i < 1024; i++) {
        if (a[i] > max_value) {
            max_value = a[i];
        }
        if (a[i] < min_value) {
            min_value = a[i];
        }
    }

    // 计算归一化的比例尺
    float scale = 500.0 / (max_value - min_value); // 将数组值映射到 [0, 500] 的范围内

    // 循环绘制曲线
    for (uint16_t i = 0; i < 1024; i++)
    {
        x = i; // x 坐标为数组的索引

        // 归一化后的 y 坐标
        y = (uint16_t)((a[i] - min_value) * scale);

        // 在LCD屏幕上的坐标 (x, y) 处绘制带有颜色的点
        // 这里假设 color 为固定值，也可以根据需要从 a[i] 中提取颜色
        color = 0xFF0000; // 假设绘制红色点
        LCD_DrawPoint(x, y, color);
    }
}



/*****************************************与王hr的fpga通信的代码*************************************************/

/**
 * @brief 转置array里的每一个元素的数据按位转置
 *
 * @param array 需要转置的数组
 */
void reverseBitsArray(uint8_t *array) {
    for (size_t i = 0; i < 16; ++i) {
        uint8_t original = array[i];
        uint8_t reversed = 0;

        // 按位倒置
        for (int j = 0; j < 8; ++j) {
            reversed |= ((original >> j) & 0x01) << (7 - j);
        }

        array[i] = reversed;
    }
}

/**
 * @brief 将从串口接收FPGA发来到的128位数据buff_array ，进行处理存到data_array
 * @param buff_array 从串口接收的原始数据
 * @param data_array 处理后的数据
 * @param data 进行运算后的结构体
 */
void get_parameter(uint8_t *buff_array, uint64_t *data_array,amfmStruct *data){
    reverseBitsArray(buff_array);
    data_array[0] = ((uint64_t)buff_array[0] << 56) |
                    ((uint64_t)buff_array[1] << 48) |
                    ((uint64_t)buff_array[2] << 40) |
                    ((uint64_t)buff_array[3] << 32) |
                    ((uint64_t)buff_array[4] << 24) |
                    ((uint64_t)buff_array[5] << 16) |
                    ((uint64_t)buff_array[6] << 8) |
                    ((uint64_t)buff_array[7]);
    data_array[1] = ((uint64_t)buff_array[8] << 56) |
                    ((uint64_t)buff_array[9] << 48) |
                    ((uint64_t)buff_array[10] << 40) |
                    ((uint64_t)buff_array[11] << 32) |
                    ((uint64_t)buff_array[12] << 24) |
                    ((uint64_t)buff_array[13] << 16) |
                    ((uint64_t)buff_array[14] << 8) |
                    ((uint64_t)buff_array[15]);
    data->type = (data_array[0] >> 61) & 0x07;
    data->phase_dev = (data_array[0] >> 19) & 0x3FFFFFFFFFF;
    data->Rb = (data_array[1] >> 50) & 0x3FFF;
    data->freq = (data_array[1] >> 36) & 0x3FFF;
    data->Min = (data_array[1] >> 22) & 0x3FFF;
    data->Max = (data_array[1] >> 8) & 0x3FFF;
    switch (data->type) {
        case 1:
            data->am.freq = (float_t)MY_FREQ / data->freq;
            data->am.ma = (float_t)(data->Max - data->Min) / (data->Max + data->Min);
            break;
        case 2:
            data->fm.freq = (float_t)MY_FREQ / data->freq;
            data->fm.mf = (float_t)data->phase_dev / 1376000000;
            data->fm.diff_fmax = data->fm.mf * data->fm.freq;
            break;
        case 5:
            data->ask.Rc = (float_t)MY_FREQ / (2*data->Rb);
            break;
        case 6:
            data->fsk.Rc = (float_t)MY_FREQ / (2*data->Rb);
            data->fsk.h = (4 * data->phase_dev) / (1376000000*PI);
            break;
        case 7:
            data->psk.Rc = (float_t)MY_FREQ / (2*data->Rb);
            break;

    }
}

