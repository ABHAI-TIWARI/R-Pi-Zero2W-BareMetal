#include "gpio.h"

// -----------------------------------------------------------------------
// gpio_set_function — configure a GPIO pin's function
//
// pin  : GPIO number (0-53)
// func : one of GPIO_FUNC_* constants (3-bit value)
//
// Each GPFSEL register controls 10 pins; pins are packed 3 bits each.
// -----------------------------------------------------------------------
void gpio_set_function(unsigned int pin, unsigned int func)
{
    // Compute byte offset of the relevant GPFSEL register (4 bytes apart)
    unsigned int reg_offset = (pin / 10u) * 4u;
    // Compute the bit-shift within that register (3 bits per pin)
    unsigned int shift      = (pin % 10u) * 3u;

    volatile uint32_t *fsel = (volatile uint32_t *)(GPIO_BASE + reg_offset);

    uint32_t val  = *fsel;
    val &= ~(0x7U << shift);            // Clear the 3-bit field
    val |=  (func & 0x7U) << shift;     // Write new function
    *fsel = val;
}

// -----------------------------------------------------------------------
// gpio_set — drive a GPIO pin high (logic 1)
// Writing a 1 to GPSETn has no effect on other pins (write-1-to-set).
// -----------------------------------------------------------------------
void gpio_set(unsigned int pin)
{
    if (pin < 32u) {
        GPSET0 = (1U << pin);
    } else {
        GPSET1 = (1U << (pin - 32u));
    }
}

// -----------------------------------------------------------------------
// gpio_clear — drive a GPIO pin low (logic 0)
// Writing a 1 to GPCLRn has no effect on other pins (write-1-to-clear).
// -----------------------------------------------------------------------
void gpio_clear(unsigned int pin)
{
    if (pin < 32u) {
        GPCLR0 = (1U << pin);
    } else {
        GPCLR1 = (1U << (pin - 32u));
    }
}

// -----------------------------------------------------------------------
// gpio_read — sample the current logic level of a GPIO pin
// Returns 1 if the pin is high, 0 if low.
// -----------------------------------------------------------------------
unsigned int gpio_read(unsigned int pin)
{
    if (pin < 32u) {
        return (GPLEV0 >> pin) & 1U;
    } else {
        return (GPLEV1 >> (pin - 32u)) & 1U;
    }
}
