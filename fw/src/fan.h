#pragma once

#include <stdint.h>
#include <stdbool.h>

#define FAN_MAX 2048

void init_fan(void);
void fan_set(uint16_t value);
void fan_sleep(bool sleep);
void fan_toggle(void);
