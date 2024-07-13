#include "myfft.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#include "math.h"




//输入 ， 输出 。 幅值
float32_t input_fft[LENGTH_SAMPLES_FFT *2] ; 
float32_t output_fft[LENGTH_SAMPLES_FFT /2] ;
float32_t Mag[LENGTH_SAMPLES_FFT];
//相位
float32_t Phase[LENGTH_SAMPLES_FFT];

uint16_t num = 0;

//复数浮点序列fft
void My_cfft_f32(void){
   
    uint16_t i;
    uint8_t fftflag = 0;//ifftFlag=0 表示正变换，ifftFlag=1 表示逆变换。
    uint8_t reverseflag = 1;//1为使能

    //按照实部，虚部，实部，虚部..... 的顺序存储数据
    /*
    for(i=0;i<LENGTH_SAMPLES_FFT;i++){
        input_fft[i*2] = 1+  cos(2*3.14159f*50 * i/1024);
        input_fft[i*2 +1] = 0;
    }*/
    
    //fft
    arm_cfft_f32(&arm_cfft_sR_f32_len1024,input_fft,fftflag,reverseflag);
    
    //arm_cmplx_mag_f32(input_fft,output_fft,1024);
    for(i=0;i<LENGTH_SAMPLES_FFT;i++){
       //printf("%f\n",output_fft[i]); 
    }
    
}

//实序列快速 FFT
void My_rfft_f32(void){
    uint16_t i;
    arm_rfft_fast_instance_f32 S;

    uint8_t ifftflag = 0;//正变换

    arm_rfft_fast_init_f32(&S, 1024);//初始化参数
    for(i=0; i<1024; i++){
        //波形是由直流分量，50Hz 正弦波组成，波形采样率 1024，初始相位 60° 
        input_fft[i] = 1 +  cos(2*3.1415926f*50*i/1024 );

    }
    arm_rfft_fast_f32(&S, input_fft, output_fft, ifftflag);//1024 点实序列快速 FFT
    arm_cmplx_mag_f32(output_fft, Mag, 1024);//这里求解了 1024 组模值，实际函数 arm_rfft_fast_f32只求解出了 512 组

    //ifftflag = 1;//逆变换
    //arm_rfft_fast_f32(&S, output_fft, input_fft, ifftflag);//1024 点实序列快速 IFFT,output是输入数据, input是输出数据

}






