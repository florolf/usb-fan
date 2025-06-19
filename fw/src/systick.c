#include <stdint.h>

#include <libopencm3/cm3/systick.h>

#include "systick.h"

volatile uint32_t ticks = 0;

void init_systick(void)
{
	systick_set_frequency(HZ, 72000000);

	systick_interrupt_enable();
	systick_counter_enable();
}

void delay_ms(uint16_t ms)
{
        delay_ticks(ms * (HZ / 1000));
}

void delay_ticks(uint16_t cnt)
{
        uint32_t timeout = ticks + cnt;

        while(time_before(ticks, timeout))
                ;
}

void sys_tick_handler(void)
{
	ticks++;
}
