#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// -----------------------------------------------------------------------
// BCM2837 / BCM2710A1 peripheral memory map
// -----------------------------------------------------------------------
#define PERIPHERAL_BASE     0x3F000000UL
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000UL)

// -----------------------------------------------------------------------
// GPIO registers
// -----------------------------------------------------------------------
#define GPFSEL0     (*(volatile uint32_t *)(GPIO_BASE + 0x00))  // Pins  0-9
#define GPFSEL1     (*(volatile uint32_t *)(GPIO_BASE + 0x04))  // Pins 10-19
#define GPFSEL2     (*(volatile uint32_t *)(GPIO_BASE + 0x08))  // Pins 20-29

#define GPSET0      (*(volatile uint32_t *)(GPIO_BASE + 0x1C))  // Set  pins  0-31
#define GPCLR0      (*(volatile uint32_t *)(GPIO_BASE + 0x28))  // Clear pins 0-31
#define GPLEV0      (*(volatile uint32_t *)(GPIO_BASE + 0x34))  // Level pins 0-31

// -----------------------------------------------------------------------
// Function-select values
// -----------------------------------------------------------------------
#define GPIO_FUNC_INPUT     0x0
#define GPIO_FUNC_OUTPUT    0x1
#define GPIO_FUNC_ALT0      0x4

// -----------------------------------------------------------------------
// RGB LED pins — common cathode, current-limiting resistors per channel
// Physical pin 11 = GPIO 17 → Blue
// Physical pin 13 = GPIO 27 → Green
// Physical pin 15 = GPIO 22 → Red
// -----------------------------------------------------------------------
#define RGB_RED_GPIO        22u
#define RGB_GREEN_GPIO      27u
#define RGB_BLUE_GPIO       17u

// -----------------------------------------------------------------------
// Tactile switch pins — 4.7k pull-up arrangement, active LOW
// Physical pin 36 = GPIO 16 → SW1 (toggle LED on/off)
// Physical pin 38 = GPIO 20 → SW2 (cycle color when LED is on)
// -----------------------------------------------------------------------
#define SW1_GPIO            16u
#define SW2_GPIO            20u

// -----------------------------------------------------------------------
// Software I2C pins — internal pull-ups present on BCM2837
// Physical pin 3 = GPIO 2 → SDA
// Physical pin 5 = GPIO 3 → SCL
// -----------------------------------------------------------------------
#define I2C_SDA_GPIO        2u
#define I2C_SCL_GPIO        3u

// -----------------------------------------------------------------------
// API
// -----------------------------------------------------------------------
void         gpio_set_function(unsigned int pin, unsigned int func);
void         gpio_set(unsigned int pin);
void         gpio_clear(unsigned int pin);
unsigned int gpio_read(unsigned int pin);

#endif // GPIO_H
