// boot.S — Startup code for Raspberry Pi Zero 2W (AArch32 / ARMv8 32-bit)
//
// The GPU firmware copies kernel.img to 0x8000 and releases all four
// Cortex-A53 cores.  We park cores 1-3 immediately and let core 0 set
// up the C runtime before calling main().

.section ".text.boot"
.global _start

_start:
    // ---------------------------------------------------------------
    // 1. Identify which CPU core we are running on.
    //    MPIDR bits [1:0] hold the CPU ID (0-3).
    //    Only core 0 continues; the rest spin in a low-power loop.
    // ---------------------------------------------------------------
    mrc     p15, 0, r1, c0, c0, 5      // Read MPIDR_EL1 (AArch32: MPIDR)
    and     r1, r1, #0x3               // Extract Aff0 field (CPU ID)
    cmp     r1, #0
    bne     .Lhalt                      // Park cores 1-3

    // ---------------------------------------------------------------
    // 2. Set up the initial stack.
    //    The linker places _start at 0x8000.  The stack grows down
    //    from there into the region below the kernel image.
    // ---------------------------------------------------------------
    ldr     sp, =_start

    // ---------------------------------------------------------------
    // 3. Zero-initialise the BSS section.
    //    The linker script exposes __bss_start and __bss_end.
    //    We write 16 bytes (four registers) per iteration for speed.
    // ---------------------------------------------------------------
    ldr     r4, =__bss_start
    ldr     r9, =__bss_end
    mov     r5, #0
    mov     r6, #0
    mov     r7, #0
    mov     r8, #0
    b       .Lbss_check

.Lbss_zero:
    stmia   r4!, {r5-r8}               // Write 16 bytes, advance pointer

.Lbss_check:
    cmp     r4, r9
    blo     .Lbss_zero                 // Loop until BSS is cleared

    // ---------------------------------------------------------------
    // 4. Branch to C main().
    //    If main() ever returns we fall through to the halt loop.
    // ---------------------------------------------------------------
    bl      main

// ---------------------------------------------------------------
// Halt: put the core into a low-power wait-for-event loop.
// ---------------------------------------------------------------
.Lhalt:
    wfe
    b       .Lhalt
