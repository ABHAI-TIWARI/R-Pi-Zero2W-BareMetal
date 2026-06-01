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
    gpio_set_function(RGB_RED_GPIO,   GPIO_FUNC_OUTPUT);
    gpio_set_function(RGB_GREEN_GPIO, GPIO_FUNC_OUTPUT);
    gpio_set_function(RGB_BLUE_GPIO,  GPIO_FUNC_OUTPUT);

    while (1) {
        /* Red */
        gpio_set(RGB_RED_GPIO);   gpio_clear(RGB_GREEN_GPIO); gpio_clear(RGB_BLUE_GPIO);
        delay(5000000u);

        /* Green */
        gpio_clear(RGB_RED_GPIO); gpio_set(RGB_GREEN_GPIO);   gpio_clear(RGB_BLUE_GPIO);
        delay(5000000u);

        /* Blue */
        gpio_clear(RGB_RED_GPIO); gpio_clear(RGB_GREEN_GPIO); gpio_set(RGB_BLUE_GPIO);
        delay(5000000u);

        /* Yellow (R+G) */
        gpio_set(RGB_RED_GPIO);   gpio_set(RGB_GREEN_GPIO);   gpio_clear(RGB_BLUE_GPIO);
        delay(5000000u);

        /* Cyan (G+B) */
        gpio_clear(RGB_RED_GPIO); gpio_set(RGB_GREEN_GPIO);   gpio_set(RGB_BLUE_GPIO);
        delay(5000000u);

        /* Magenta (R+B) */
        gpio_set(RGB_RED_GPIO);   gpio_clear(RGB_GREEN_GPIO); gpio_set(RGB_BLUE_GPIO);
        delay(5000000u);

        /* White (R+G+B) */
        gpio_set(RGB_RED_GPIO);   gpio_set(RGB_GREEN_GPIO);   gpio_set(RGB_BLUE_GPIO);
        delay(5000000u);
    }
}
