#include "test.h"
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