# -----------------------------------------------------------------------
# Makefile — Raspberry Pi Zero 2W bare-metal (AArch32, arm-none-eabi)
# -----------------------------------------------------------------------

# Cross-compilation toolchain prefix
CROSS   := arm-none-eabi-

CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE    := $(CROSS)size

# -----------------------------------------------------------------------
# Compiler flags
#   -mcpu=cortex-a53        : target the Cortex-A53 inside BCM2710A1
#   -mfloat-abi=hard        : hardware floating-point ABI
#   -mfpu=neon-fp-armv8     : NEON + VFPv4 unit present on A53
#   -ffreestanding          : no standard library startup/main assumption
#   -nostdlib               : do not link against libc/libm/etc.
#   -nostartfiles           : do not include crt0 / crti / crtend
# -----------------------------------------------------------------------
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

# -----------------------------------------------------------------------
# Output directories
# -----------------------------------------------------------------------
OBJ_DIR := obj
IMG_DIR := img

# -----------------------------------------------------------------------
# Sources and derived object list
# -----------------------------------------------------------------------
C_SRCS   := src/main.c src/gpio.c
ASM_SRCS := src/boot.S

OBJS := $(patsubst src/%.c,  $(OBJ_DIR)/%.o, $(C_SRCS)) \
        $(patsubst src/%.S,  $(OBJ_DIR)/%.o, $(ASM_SRCS))

TARGET := kernel

# -----------------------------------------------------------------------
# Top-level targets
# -----------------------------------------------------------------------
.PHONY: all clean dump

all: $(IMG_DIR)/$(TARGET).img

# Create output directories if they don't exist
$(OBJ_DIR) $(IMG_DIR):
	mkdir -p $@

# Link all objects into an ELF executable
$(IMG_DIR)/$(TARGET).elf: $(OBJS) linker.ld | $(OBJ_DIR) $(IMG_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

# Strip to a raw binary — this is what the RPi firmware boots
$(IMG_DIR)/$(TARGET).img: $(IMG_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	cp config.txt $(IMG_DIR)/config.txt
	@echo ""
	@echo "  Build complete: $@"
	@echo "  SD card files ready in $(IMG_DIR)/"
	@echo ""

# -----------------------------------------------------------------------
# Compile / assemble rules
# -----------------------------------------------------------------------
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Use GCC to assemble .S files so C preprocessor macros work if needed
$(OBJ_DIR)/%.o: src/%.S | $(OBJ_DIR)
	$(CC) $(ASFLAGS) -o $@ $<

# -----------------------------------------------------------------------
# Convenience targets
# -----------------------------------------------------------------------

# Disassembly listing — useful for debugging or verifying code placement
dump: $(IMG_DIR)/$(TARGET).elf
	$(OBJDUMP) -d -S $< > $(OBJ_DIR)/$(TARGET).dump
	@echo "Disassembly written to $(OBJ_DIR)/$(TARGET).dump"

clean:
	rm -rf $(OBJ_DIR) $(IMG_DIR)
