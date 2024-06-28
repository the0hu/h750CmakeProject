#include "test.h"
#include <stdio.h>
#include <string.h>
#include <usart.h>
#include "arm_math.h"
#include <ad9910.h>
#include <stdint.h>

/**
 * @brief 对fft求模后的一半的数组进行处理，找出两个最大值以及索引位置
 *
 * @param FftMagStruct 结构体
 */
extern void Test(FftMagStruct *data) {

    uint16_t max1=0,max2=0,index1=0,index2=0;

    data ->wave_a = data ->mag[0];
    data ->wave_b = data ->mag[0];
    data ->index_a = 0;
    data ->index_b = 0;
    data ->freq_a = 0;
    data ->freq_b = 0;

    //波形a的频率小于b的频率
    for (int i =1; i<MAGLENGTH; i++) {
        if (data->mag[i] > max1) {
            max2 = max1;
            index2 = index1;

            max1 = data->mag[i];
            index1 = i;
        }
        else if (data->mag[i] > max2) {
            max2 = data->mag[i];
            index2 = i;
        }
    }
    //当max1的位置在max2后面，即max1的频率高于max2
    //因为波形b的频率高于a
    if (index2<index1) {
        data->wave_a = max2;
        data->index_a = index2;
        data->wave_b = max1;
        data->index_b = index1;
    }
    else if (index1 < index2) {
        data->wave_a = max1;
        data->index_a = index1;
        data->wave_b = max2;
        data->index_b = index2;
    }
    data->freq_a = (float32_t)SAMP_FREQ / (MAGLENGTH*2) * data->index_a;
    data->freq_b = (float32_t)SAMP_FREQ / (MAGLENGTH*2) * data->index_b;

    jude_wave(data);
    
}


/**
 * @brief 判断波形，改变data->type的值，规定0为正弦波，1为三角波;
 *
 * @param FftMagStruct 结构体
 */
void jude_wave(FftMagStruct *data){
    float32_t temp =0 , temp5=0;
    uint8_t n=6;
    uint8_t times = 4;//times次谐波
    //在a波的3倍频附近找出最大值
    for (int i= ( 3*data->index_a - n) ;i<(3*data->index_a + n) ; i++) {
        if (data->mag[i] > temp) {
            temp = data->mag[i];
        }
    }
    //在a波的6倍频附近找出最大值，从而判断是否为三角波
    if ((times*data->index_a+n) >= MAGLENGTH) {
        for (int i= ( times*data->index_a - n) ;i<MAGLENGTH ; i++) {
            if (data->mag[i] > temp5) {
                temp5 = data->mag[i];
            }
        }
    }
    else {
        for (int i= ( times*data->index_a - n) ;i<(times*data->index_a + n) ; i++) {
            if (data->mag[i] > temp5) {
                temp5 = data->mag[i];
            }
        }
    } 
    if (temp > 30 && temp5 >10) 
        data->type_a = 1; //1为三角波
    else if(temp < 30)
        data->type_a = 0 ;
    temp=0;temp5=0;
    /*******************************************************/
    
    //在b波的3倍频附近找出最大值
    for (int i= ( 3*data->index_b - n) ;i<(3*data->index_b + n) ; i++) {
        if (data->mag[i] > temp) {
            temp = data->mag[i];
        }
    }
    //在b波的times倍频附近找出最大值，从而判断是否为三角波
    if ((times*data->index_b+n) >= MAGLENGTH) {
        for (int i= ( times*data->index_b - n) ;i<MAGLENGTH ; i++) {
            if (data->mag[i] > temp5) {
                temp5 = data->mag[i];
            }
        }
    }
    else {
        for (int i= ( times*data->index_b - n) ;i<(times*data->index_b + n) ; i++) {
            if (data->mag[i] > temp5) {
                temp5 = data->mag[i];
            }
        }
    } 
    if (temp > 30 && temp5 >10) 
        data->type_b = 1; //1为三角波
    else if(temp < 30)
        data->type_b = 0 ;

}




void Slidingwindow_filter(uint16_t *input,uint16_t size,uint16_t windowsize,uint16_t *output){

    // uint16_t sum = 0;
    // for (int i=0; i<windowsize; i++) {
    //     sum += input[i];
    // }
    // output[0] = sum / windowsize;

    // for (uint16_t i=windowsize; i <size; i++) {
    //     sum += input[i] -input[i - windowsize];
    //     output[i] = sum /windowsize; 
    // }
}



extern void my_print(const char *ch ){
    HAL_UART_Transmit(&huart2, (uint8_t *)ch, strlen(ch), 0xff);
  
}

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