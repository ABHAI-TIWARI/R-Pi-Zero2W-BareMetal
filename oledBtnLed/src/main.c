#include "gpio.h"
#include "i2c.h"
#include "ssd1306.h"

// -----------------------------------------------------------------------
// Colour table
// -----------------------------------------------------------------------
#define NUM_COLOURS  7u

static const unsigned char colour_table[NUM_COLOURS][3] = {
    { 1, 0, 0 },    // 0 — Red
    { 0, 1, 0 },    // 1 — Green
    { 0, 0, 1 },    // 2 — Blue
    { 1, 1, 0 },    // 3 — Yellow
    { 0, 1, 1 },    // 4 — Cyan
    { 1, 0, 1 },    // 5 — Magenta
    { 1, 1, 1 },    // 6 — White
};

// -----------------------------------------------------------------------
// OLED display strings
//
// Layout (128×64, 8 pages of 8px each):
//
//   Page 0 : "LED Controller"   — header, never changes
//   Page 3 : "Status: ON " / "Status: OFF"  — 11 chars, col 31
//   Page 5 : "Color: Red    " … "Color: Magenta" — 14 chars, col 22
//
// All strings on a given row are the same width so overwriting works
// without needing to clear the page first.
//
// Centering:
//   "LED Controller" = 14 × 6 = 84 px → col (128-84)/2 = 22
//   "Status: ON "    = 11 × 6 = 66 px → col (128-66)/2 = 31
//   "Color: Magenta" = 14 × 6 = 84 px → col (128-84)/2 = 22
// -----------------------------------------------------------------------

#define HDR_COL     22u
#define HDR_PAGE     0u
#define STAT_COL    31u
#define STAT_PAGE    3u
#define CLR_COL     22u
#define CLR_PAGE     5u

// All color strings are padded to 14 characters (same width as "Color: Magenta")
static const char *colour_str[NUM_COLOURS] = {
    "Color: Red    ",
    "Color: Green  ",
    "Color: Blue   ",
    "Color: Yellow ",
    "Color: Cyan   ",
    "Color: Magenta",
    "Color: White  ",
};

// -----------------------------------------------------------------------
// Debounce — ~20 ms at 600 MHz (375 000 ticks × ~4 cycles ≈ 2.5 ms)
// -----------------------------------------------------------------------
#define DEBOUNCE_TICKS  375000u

// -----------------------------------------------------------------------
// delay — simple busy-wait loop
// -----------------------------------------------------------------------
static void delay(unsigned int ticks)
{
    volatile unsigned int i;
    for (i = 0u; i < ticks; i++) {
        __asm__ volatile("nop");
    }
}

// -----------------------------------------------------------------------
// led_apply — drive RGB LED to the current colour, or turn all channels off
// -----------------------------------------------------------------------
static void led_apply(unsigned int on, unsigned int idx)
{
    if (on) {
        if (colour_table[idx][0]) gpio_set(RGB_RED_GPIO);   else gpio_clear(RGB_RED_GPIO);
        if (colour_table[idx][1]) gpio_set(RGB_GREEN_GPIO); else gpio_clear(RGB_GREEN_GPIO);
        if (colour_table[idx][2]) gpio_set(RGB_BLUE_GPIO);  else gpio_clear(RGB_BLUE_GPIO);
    } else {
        gpio_clear(RGB_RED_GPIO);
        gpio_clear(RGB_GREEN_GPIO);
        gpio_clear(RGB_BLUE_GPIO);
    }
}

// -----------------------------------------------------------------------
// oled_update — refresh the Status and Color rows on the OLED
// -----------------------------------------------------------------------
static void oled_update(unsigned int on, unsigned int idx)
{
    if (on) {
        ssd1306_draw_string(STAT_COL, STAT_PAGE, "Status: ON ");
        ssd1306_draw_string(CLR_COL,  CLR_PAGE,  colour_str[idx]);
    } else {
        ssd1306_draw_string(STAT_COL, STAT_PAGE, "Status: OFF");
        ssd1306_draw_string(CLR_COL,  CLR_PAGE,  "              ");  // 14 spaces
    }
}

// -----------------------------------------------------------------------
// main
// -----------------------------------------------------------------------
void main(void)
{
    // ---------------------------------------------------------------
    // GPIO setup
    // ---------------------------------------------------------------
    gpio_set_function(RGB_RED_GPIO,   GPIO_FUNC_OUTPUT);
    gpio_set_function(RGB_GREEN_GPIO, GPIO_FUNC_OUTPUT);
    gpio_set_function(RGB_BLUE_GPIO,  GPIO_FUNC_OUTPUT);

    gpio_set_function(SW1_GPIO, GPIO_FUNC_INPUT);
    gpio_set_function(SW2_GPIO, GPIO_FUNC_INPUT);

    // ---------------------------------------------------------------
    // Initial state — LED off, colour index at Red (ready for first ON)
    // ---------------------------------------------------------------
    unsigned int led_on     = 0u;
    unsigned int colour_idx = 0u;

    led_apply(led_on, colour_idx);

    // ---------------------------------------------------------------
    // Power-on delay and OLED init
    // ---------------------------------------------------------------
    delay(300000u);  // ~100 ms for rails to stabilise

    i2c_init();
    ssd1306_init();
    ssd1306_clear();

    // Draw static header (never changes)
    ssd1306_draw_string(HDR_COL, HDR_PAGE, "LED Controller");

    // Draw initial status
    oled_update(led_on, colour_idx);

    // ---------------------------------------------------------------
    // Main loop
    // ---------------------------------------------------------------
    while (1) {

        // -----------------------------------------------------------
        // SW1 — Toggle LED on / off
        // -----------------------------------------------------------
        if (gpio_read(SW1_GPIO) == 0u) {
            delay(DEBOUNCE_TICKS);

            if (gpio_read(SW1_GPIO) == 0u) {
                led_on ^= 1u;
                led_apply(led_on, colour_idx);
                oled_update(led_on, colour_idx);

                // Wait for release
                while (gpio_read(SW1_GPIO) == 0u) {}
                delay(DEBOUNCE_TICKS);
            }
        }

        // -----------------------------------------------------------
        // SW2 — Cycle colour (only active when LED is on)
        // -----------------------------------------------------------
        if (led_on && (gpio_read(SW2_GPIO) == 0u)) {
            delay(DEBOUNCE_TICKS);

            if (gpio_read(SW2_GPIO) == 0u) {
                colour_idx = (colour_idx + 1u) % NUM_COLOURS;
                led_apply(led_on, colour_idx);
                ssd1306_draw_string(CLR_COL, CLR_PAGE, colour_str[colour_idx]);

                // Wait for release
                while (gpio_read(SW2_GPIO) == 0u) {}
                delay(DEBOUNCE_TICKS);
            }
        }
    }
}
