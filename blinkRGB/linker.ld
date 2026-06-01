/*
 * linker.ld — Linker script for Raspberry Pi Zero 2W bare-metal (AArch32)
 *
 * The GPU firmware loads kernel.img to physical address 0x8000 and
 * begins execution there.  The stack grows downward from 0x8000 into
 * the region below the image (set up in boot.S).
 */

ENTRY(_start)

SECTIONS
{
    /*
     * Place everything starting at the GPU firmware load address.
     * Changing this requires a matching kernel_address= in config.txt.
     */
    . = 0x8000;

    /*
     * .text — executable code
     * The .text.boot section (boot.S) MUST be first so that _start sits
     * exactly at 0x8000, which is the address the CPU jumps to.
     */
    .text :
    {
        KEEP(*(.text.boot))     /* boot.S — must be at 0x8000 */
        *(.text*)               /* all other code              */
    }

    /* .rodata — read-only constants and string literals */
    .rodata : ALIGN(4)
    {
        *(.rodata*)
    }

    /* .data — initialised global/static variables */
    .data : ALIGN(4)
    {
        *(.data*)
    }

    /*
     * .bss — zero-initialised global/static variables
     * boot.S loops from __bss_start to __bss_end and writes zeros.
     */
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
