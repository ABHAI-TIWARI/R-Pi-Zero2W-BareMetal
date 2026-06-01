/*
 * linker.ld — Linker script for Raspberry Pi Zero 2W bare-metal (AArch32)
 */

ENTRY(_start)

SECTIONS
{
    . = 0x8000;

    .text :
    {
        KEEP(*(.text.boot))
        *(.text*)
    }

    .rodata : ALIGN(4)
    {
        *(.rodata*)
    }

    .data : ALIGN(4)
    {
        *(.data*)
    }

    . = ALIGN(4);
    __bss_start = .;

    .bss :
    {
        *(.bss*)
        *(COMMON)
    }

    . = ALIGN(4);
    __bss_end = .;
}
