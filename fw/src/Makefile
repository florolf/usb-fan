TOPDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJ = $(patsubst %.c,%.o,$(wildcard *.c))

CROSS_PREFIX ?= arm-none-eabi-
CC=$(CROSS_PREFIX)gcc
OBJCOPY=$(CROSS_PREFIX)objcopy
SIZE=$(CROSS_PREFIX)size

CFLAGS = -Wall -Wextra -std=gnu99 -I../libopencm3/include -I ../common -DSTM32F0 -mcpu=cortex-m0 -mthumb -fno-common -ffreestanding -flto
LDFLAGS = -lopencm3_stm32f0 -L../libopencm3/lib -mthumb -mcpu=cortex-m0 -nostartfiles -Wl,--gc-sections -flto -T linker.ld

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -Werror -Wno-error=unused-function -Og -g
#	LDFLAGS += -g
else
	CFLAGS += -O3
endif

all: firmware.bin

firmware.elf: $(OBJ)
	$(CC) -o firmware.elf $^ $(LDFLAGS)
	$(SIZE) -B firmware.elf

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

flash: firmware.bin
	echo "reset halt; flash write_image erase unlock $(TOPDIR)/firmware.bin 0x8000000; reset" | nc 127.0.0.1 4444

clean:
	rm -f *.o firmware.elf firmware.bin

debug:
	arm-none-eabi-gdb  -ex 'target extended-remote :2331' firmware.elf

.PHONY: all flash clean debug
