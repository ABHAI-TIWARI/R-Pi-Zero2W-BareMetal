#include "gpio.h"

// -----------------------------------------------------------------------
// delay — busy-wait for approximately `ticks` NOP cycles.
//
// This is intentionally simple.  At 1 GHz clock, 500 000 NOPs ≈ 0.5 ms.
// Adjust the constant in main() to taste once UART or a timer driver
// provides accurate timekeeping.
// -----------------------------------------------------------------------
static void delay(unsigned int ticks)
{
    volatile unsigned int i;
    for (i = 0u; i < ticks; i++) {
        __asm__ volatile("nop");
    }
}

// -----------------------------------------------------------------------
// main — entry point called from boot.S after CPU + BSS initialisation.
//
// Blinks the green ACT LED (GPIO 29) at ~1 Hz.
// -----------------------------------------------------------------------
void main(void)
{
    gpio_set_function(ACT_LED_GPIO, GPIO_FUNC_OUTPUT);

    while (1) {
        gpio_set(ACT_LED_GPIO);         // LED ON
        delay(5000000u);              // ~1 s at 600 MHz

        gpio_clear(ACT_LED_GPIO);       // LED OFF
        delay(5000000u);              // ~1 s at 600 MHz
    }
}
