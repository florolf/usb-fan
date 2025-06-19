#pragma once

#include <stdint.h>

extern volatile uint32_t ticks;

void init_systick(void);
void delay_ms(uint16_t ms);
void delay_ticks(uint16_t cnt);

#define HZ 1000
#define time_before(x,y) (((int32_t)(y) - (int32_t)(x)) > 0)
#define time_after(x,y) time_before(y,x)
