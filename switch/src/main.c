#include "gpio.h"

// -----------------------------------------------------------------------
// Colour table — RGB states for each colour step
// Each entry: { red, green, blue }  1 = ON, 0 = OFF
// -----------------------------------------------------------------------
#define NUM_COLOURS  7u

static const unsigned char colour_table[NUM_COLOURS][3] = {
    { 1, 0, 0 },    // 0 — Red
    { 0, 1, 0 },    // 1 — Green
    { 0, 0, 1 },    // 2 — Blue
    { 1, 1, 0 },    // 3 — Yellow  (R+G)
    { 0, 1, 1 },    // 4 — Cyan    (G+B)
    { 1, 0, 1 },    // 5 — Magenta (R+B)
    { 1, 1, 1 },    // 6 — White   (R+G+B)
};

// -----------------------------------------------------------------------
// set_colour — drive the RGB LED to the given colour index
// -----------------------------------------------------------------------
static void set_colour(unsigned int idx)
{
    if (colour_table[idx][0]) gpio_set(RGB_RED_GPIO);   else gpio_clear(RGB_RED_GPIO);
    if (colour_table[idx][1]) gpio_set(RGB_GREEN_GPIO); else gpio_clear(RGB_GREEN_GPIO);
    if (colour_table[idx][2]) gpio_set(RGB_BLUE_GPIO);  else gpio_clear(RGB_BLUE_GPIO);
}

// -----------------------------------------------------------------------
// delay — busy-wait NOP loop (~1 ms at 600 MHz per 150 000 ticks)
// -----------------------------------------------------------------------
static void delay(unsigned int ticks)
{
    volatile unsigned int i;
    for (i = 0u; i < ticks; i++) {
        __asm__ volatile("nop");
    }
}

// -----------------------------------------------------------------------
// debounce_delay — wait ~20 ms for switch bounce to settle
// 3 000 000 ticks * 4 cycles / 600 MHz ≈ 20 ms
// -----------------------------------------------------------------------
#define DEBOUNCE_TICKS  375000u

// -----------------------------------------------------------------------
// main — entry point
//
// Behaviour:
//   SW1 (GPIO 16, active LOW) — step to next colour
//   SW2 (GPIO 20, active LOW) — step to previous colour
//
// Debounce strategy:
//   1. Detect falling edge (pin reads LOW).
//   2. Wait DEBOUNCE_TICKS to let bounce settle.
//   3. Confirm pin is still LOW (valid press).
//   4. Change colour.
//   5. Wait for pin to go HIGH again (release) before accepting next press.
// -----------------------------------------------------------------------
void main(void)
{
    // Configure RGB LED pins as outputs
    gpio_set_function(RGB_RED_GPIO,   GPIO_FUNC_OUTPUT);
    gpio_set_function(RGB_GREEN_GPIO, GPIO_FUNC_OUTPUT);
    gpio_set_function(RGB_BLUE_GPIO,  GPIO_FUNC_OUTPUT);

    // Configure switch pins as inputs (pull-ups are external 4.7k resistors)
    gpio_set_function(SW1_GPIO, GPIO_FUNC_INPUT);
    gpio_set_function(SW2_GPIO, GPIO_FUNC_INPUT);

    // Start with Red
    unsigned int colour_idx = 0u;
    set_colour(colour_idx);

    while (1) {
        // ---------------------------------------------------------------
        // Poll SW1 — Next colour
        // ---------------------------------------------------------------
        if (gpio_read(SW1_GPIO) == 0u) {
            delay(DEBOUNCE_TICKS);                  // wait for bounce to settle

            if (gpio_read(SW1_GPIO) == 0u) {        // confirm still pressed
                colour_idx = (colour_idx + 1u) % NUM_COLOURS;
                set_colour(colour_idx);

                // Wait for release
                while (gpio_read(SW1_GPIO) == 0u) { /* spin */ }
                delay(DEBOUNCE_TICKS);              // debounce on release
            }
        }

        // ---------------------------------------------------------------
        // Poll SW2 — Previous colour
        // ---------------------------------------------------------------
        if (gpio_read(SW2_GPIO) == 0u) {
            delay(DEBOUNCE_TICKS);

            if (gpio_read(SW2_GPIO) == 0u) {
                colour_idx = (colour_idx == 0u) ? (NUM_COLOURS - 1u) : (colour_idx - 1u);
                set_colour(colour_idx);

                // Wait for release
                while (gpio_read(SW2_GPIO) == 0u) { /* spin */ }
                delay(DEBOUNCE_TICKS);
            }
        }
    }
}
