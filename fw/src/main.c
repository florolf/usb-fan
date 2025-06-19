#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/syscfg.h>

#include "usb.h"
#include "systick.h"
#include "fan.h"

static void init_clocks(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_TIM14);
	rcc_periph_clock_enable(RCC_SYSCFG_COMP);

	rcc_periph_reset_pulse(RST_GPIOA);
	rcc_periph_reset_pulse(RST_TIM3);
	rcc_periph_reset_pulse(RST_TIM14);
}

void __attribute__ ((noreturn)) enter_bootloader(void)
{
	uint32_t sp, pc;

	sp = *(uint32_t*)0x1fffc400;
	pc = *(uint32_t*)0x1fffc404;

	__asm__ volatile ("msr msp, %0" : : "r" (sp));
	((void(*)(void))pc)();

	while (1)
		;
}

extern unsigned _ebss;
uint32_t *bootloader_magic = (uint32_t*)&_ebss;

void main(void)
{
	if (*bootloader_magic == 0xb007b007) {
		*bootloader_magic = 0;
		enter_bootloader();
	}

	rcc_clock_setup_in_hsi48_out_48mhz();
	crs_autotrim_usb_enable();
	rcc_set_usbclk_source(RCC_HSI48);

	init_systick();
	init_clocks();

	// LED
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set(GPIOA, GPIO1);

	// Button
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO4);

	init_fan();

	// map PA11 and PA12 to pins 17 and 18 for USB
	SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
	init_usb();

	uint32_t last_change = 0;
	bool last_state = false;
	bool new_press = true;

	while (1) {
		usb_poll();

		bool state = !gpio_get(GPIOA, GPIO4);
		if (state != last_state) {
			last_state = state;
			last_change = ticks;
			continue;
		}

		last_state = state;

		if (!time_after(ticks, last_change + HZ/50))
			continue;

		if (!state) {
			new_press = true;
			continue;
		}

		if (!new_press)
			continue;

		new_press = false;
		fan_toggle();
	}
}
