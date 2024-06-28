#ifndef __TEST_H__
#define __TEST_H__

#include <main.h>
#include <stdint.h>

extern void my_print(const char *ch );
void sendDataViaUSART(const uint32_t *data, int dataSize);
#endif
