#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define PERIPHERAL_BASE     0x3F000000UL
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000UL)

#define GPFSEL0     (*(volatile uint32_t *)(GPIO_BASE + 0x00))
#define GPFSEL1     (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPFSEL2     (*(volatile uint32_t *)(GPIO_BASE + 0x08))

#define GPIO_FUNC_INPUT     0x0
#define GPIO_FUNC_OUTPUT    0x1
#define GPIO_FUNC_ALT0      0x4
#define GPIO_FUNC_ALT1      0x5
#define GPIO_FUNC_ALT2      0x6
#define GPIO_FUNC_ALT3      0x7
#define GPIO_FUNC_ALT4      0x3
#define GPIO_FUNC_ALT5      0x2

// I2C1 pins — GPIO 2 = SDA1 (ALT0), GPIO 3 = SCL1 (ALT0)
#define I2C_SDA_GPIO        2u   // Physical pin 3
#define I2C_SCL_GPIO        3u   // Physical pin 5

void gpio_set_function(unsigned int pin, unsigned int func);

#endif // GPIO_H
