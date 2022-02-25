#ifndef _PIT_H_
#define _PIT_H_

#include "lib.h"

#define PIT_IRQ         0
#define PIT_CMD     0x43
#define PIT_DATA    0x40       // PIT ports
#define PIT_MODE      0x36    // 00110110 Channel 0,  Access mode: lobyte/hibyte , Mode 3 (square wave generator)
#define PIT_INTV_L    0x9C   // 1.19318 MHz->100hz=11932 = 0x2E9C
#define PIT_INTV_H    0x2E   //

void pit_init();
void pit_interrupt();

#endif
