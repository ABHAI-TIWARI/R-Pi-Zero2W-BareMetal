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
// Blinks the green ACT LED (GPIO 29) at roughly 1 Hz:
//   ON  500 000 ticks  → ~0.5 s
//   OFF 500 000 ticks  → ~0.5 s
// -----------------------------------------------------------------------
void main(void)
{
    gpio_set_function(ACT_LED_GPIO, GPIO_FUNC_OUTPUT);

    while (1) {
        gpio_set(ACT_LED_GPIO);         // LED ON
        delay(500000u);

        gpio_clear(ACT_LED_GPIO);       // LED OFF
        delay(500000u);
    }
}
