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
# Sources and derived object list
# -----------------------------------------------------------------------
C_SRCS   := src/main.c src/gpio.c
ASM_SRCS := src/boot.S

OBJS := $(C_SRCS:.c=.o) $(ASM_SRCS:.S=.o)

TARGET := kernel

# -----------------------------------------------------------------------
# Top-level targets
# -----------------------------------------------------------------------
.PHONY: all clean dump

all: $(TARGET).img

# Link all objects into an ELF executable
$(TARGET).elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

# Strip to a raw binary — this is what the RPi firmware boots
$(TARGET).img: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo ""
	@echo "  Build complete: $@"
	@echo "  Copy $@ to the SD card root as kernel.img"
	@echo ""

# -----------------------------------------------------------------------
# Compile / assemble rules
# -----------------------------------------------------------------------
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Use GCC to assemble .S files so C preprocessor macros work if needed
src/%.o: src/%.S
	$(CC) $(ASFLAGS) -o $@ $<

# -----------------------------------------------------------------------
# Convenience targets
# -----------------------------------------------------------------------

# Disassembly listing — useful for debugging or verifying code placement
dump: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).dump
	@echo "Disassembly written to $(TARGET).dump"

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).img $(TARGET).dump
