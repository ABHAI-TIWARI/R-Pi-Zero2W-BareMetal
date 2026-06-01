#include "gpio.h"
#include "i2c.h"
#include "ssd1306.h"

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
// main
//
// Initialises I2C1, SSD1306 OLED display, clears the screen, then
// prints "Hello ABHAI.." centred on page 3 (vertical middle).
//
// "Hello ABHAI.." = 13 characters × 6 pixels = 78 pixels wide.
// Centre column = (128 - 78) / 2 = 25.
// -----------------------------------------------------------------------
void main(void)
{
    // Allow power rails to stabilise after boot (~100 ms at 600 MHz)
    delay(300000u);

    i2c_init();
    ssd1306_init();
    ssd1306_clear();

    // Draw "Hello ABHAI.." centred on the display
    ssd1306_draw_string(25u, 3u, "Hello ABHAI..");

    // Nothing more to do — loop forever
    while (1) {
        __asm__ volatile("wfe");
    }
}
