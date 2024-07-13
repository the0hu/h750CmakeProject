#ifndef __TEST_H__
#define __TEST_H__

#include <main.h>
#include <math.h>
#include <stdint.h>

#define MY_FREQ 1800000
#define PI 3.14159

typedef struct {
    float_t freq;
    float_t ma;
}am_struct;

typedef struct{
    float_t freq;
    float_t mf;
    float_t diff_fmax;
}fm_struct ;

typedef struct{
    float_t Rc;

}ask_struct;

typedef struct{
    float_t Rc;
    float_t h;
}fsk_strcut ;

typedef struct{
    float_t Rc;
}psk_strcut ;
typedef struct{
    uint8_t type;
    uint64_t phase_dev; //最大相偏
    uint16_t Rb,freq, Min, Max;
    am_struct am;
    fm_struct fm;
    ask_struct ask;
    fsk_strcut fsk;
    psk_strcut psk;

}amfmStruct;



extern void my_print(const char *ch );
void sendDataViaUSART(const uint32_t *data, int dataSize);
void reverseBitsArray(uint8_t *array);
void get_parameter(uint8_t *buff_array, uint64_t *data_array,amfmStruct *data);

int32_t mapToRange(uint16_t adcValue);
void ad9910_fm();

#endif
