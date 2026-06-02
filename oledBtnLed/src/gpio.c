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
    GPSET0 = (1U << pin);
}

// -----------------------------------------------------------------------
// gpio_clear — drive a GPIO pin low
// -----------------------------------------------------------------------
void gpio_clear(unsigned int pin)
{
    GPCLR0 = (1U << pin);
}

// -----------------------------------------------------------------------
// gpio_read — sample the current logic level of a GPIO pin
// Returns 1 if high, 0 if low.
// -----------------------------------------------------------------------
unsigned int gpio_read(unsigned int pin)
{
    return (GPLEV0 >> pin) & 1U;
}
