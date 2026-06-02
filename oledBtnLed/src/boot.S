// boot.S — Startup code for Raspberry Pi Zero 2W (AArch32 / ARMv8 32-bit)

.section ".text.boot"
.global _start

_start:
    mrc     p15, 0, r1, c0, c0, 5
    and     r1, r1, #0x3
    cmp     r1, #0
    bne     .Lhalt

    ldr     sp, =_start

    ldr     r4, =__bss_start
    ldr     r9, =__bss_end
    mov     r5, #0
    mov     r6, #0
    mov     r7, #0
    mov     r8, #0
    b       .Lbss_check

.Lbss_zero:
    stmia   r4!, {r5-r8}

.Lbss_check:
    cmp     r4, r9
    blo     .Lbss_zero

    bl      main

.Lhalt:
    wfe
    b       .Lhalt
