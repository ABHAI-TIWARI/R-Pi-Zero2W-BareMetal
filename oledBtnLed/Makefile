# -----------------------------------------------------------------------
# Makefile — oledBtnLed  (Raspberry Pi Zero 2W bare-metal, AArch32)
# -----------------------------------------------------------------------

CROSS   := arm-none-eabi-
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE    := $(CROSS)size

CPU_FLAGS := -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8

CFLAGS := \
    $(CPU_FLAGS)      \
    -ffreestanding    \
    -nostdlib         \
    -nostartfiles     \
    -O2               \
    -Wall             \
    -Wextra           \
    -I src

ASFLAGS := $(CPU_FLAGS) -c

LDFLAGS := -T linker.ld --no-undefined

OBJ_DIR := obj
IMG_DIR := img

C_SRCS   := src/main.c src/gpio.c src/i2c.c src/ssd1306.c
ASM_SRCS := src/boot.S

OBJS := $(patsubst src/%.c,  $(OBJ_DIR)/%.o, $(C_SRCS)) \
        $(patsubst src/%.S,  $(OBJ_DIR)/%.o, $(ASM_SRCS))

TARGET := kernel

.PHONY: all clean dump

all: $(IMG_DIR)/$(TARGET).img

$(OBJ_DIR) $(IMG_DIR):
	mkdir -p $@

$(IMG_DIR)/$(TARGET).elf: $(OBJS) linker.ld | $(OBJ_DIR) $(IMG_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

$(IMG_DIR)/$(TARGET).img: $(IMG_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	cp config.txt $(IMG_DIR)/config.txt
	@echo ""
	@echo "  Build complete: $@"
	@echo "  SD card files ready in $(IMG_DIR)/"
	@echo ""

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: src/%.S | $(OBJ_DIR)
	$(CC) $(ASFLAGS) -o $@ $<

dump: $(IMG_DIR)/$(TARGET).elf
	$(OBJDUMP) -d -S $< > $(OBJ_DIR)/$(TARGET).dump
	@echo "Disassembly written to $(OBJ_DIR)/$(TARGET).dump"

clean:
	rm -rf $(OBJ_DIR) $(IMG_DIR)
