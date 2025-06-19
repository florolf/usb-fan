#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>

#include "fan.h"

static uint16_t fan_speed;

static void _fan_set(uint16_t value)
{
	if (value == 0) {
		gpio_clear(GPIOA, GPIO5);
		gpio_set(GPIOA, GPIO1);
	} else {
		gpio_set(GPIOA, GPIO5);
		gpio_clear(GPIOA, GPIO1);
	}

	if (value > FAN_MAX)
		value = FAN_MAX;

	timer_set_oc_value(TIM3, TIM_OC1, FAN_MAX - value);
}

void fan_set(uint16_t value)
{
	fan_speed = value;

	_fan_set(fan_speed);
}

void fan_sleep(bool sleep)
{
	_fan_set(sleep ? 0 : fan_speed);
}

void fan_toggle(void)
{
	if (fan_speed)
		fan_set(0);
	else
		fan_set(FAN_MAX);
}

void init_fan(void)
{
	// V_FAN control
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO5);

	// PWM out
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	gpio_set_af(GPIOA, GPIO_AF1, GPIO6);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, GPIO6);

	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_period(TIM3, FAN_MAX - 1);

	timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);
	timer_enable_oc_output(TIM3, TIM_OC1);
	timer_enable_oc_preload(TIM3, TIM_OC1);

	fan_set(FAN_MAX);
	timer_enable_counter(TIM3);

	// Tacho in
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
	gpio_set_af(GPIOA, GPIO_AF4, GPIO7);
}
