#include "gpio.h"

// -----------------------------------------------------------------------
// gpio_set_function — configure a GPIO pin's function
// -----------------------------------------------------------------------
void gpio_set_function(unsigned int pin, unsigned int func)
{
    unsigned int reg_offset = (pin / 10u) * 4u;
    unsigned int shift      = (pin % 10u) * 3u;

    volatile uint32_t *fsel = (volatile uint32_t *)(GPIO_BASE + reg_offset);

    uint32_t val  = *fsel;
    val &= ~(0x7U << shift);
    val |=  (func & 0x7U) << shift;
    *fsel = val;
}

// -----------------------------------------------------------------------
// gpio_set — drive a GPIO pin high
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
// gpio_clear — drive a GPIO pin low
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
// Returns 1 if high, 0 if low.
// -----------------------------------------------------------------------
unsigned int gpio_read(unsigned int pin)
{
    if (pin < 32u) {
        return (GPLEV0 >> pin) & 1U;
    } else {
        return (GPLEV1 >> (pin - 32u)) & 1U;
    }
}
