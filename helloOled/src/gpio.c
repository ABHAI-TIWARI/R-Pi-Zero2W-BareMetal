#include "gpio.h"

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
