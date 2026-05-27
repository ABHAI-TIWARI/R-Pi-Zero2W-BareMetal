#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// -----------------------------------------------------------------------
// BCM2837 / BCM2710A1 peripheral memory map
// Raspberry Pi Zero 2W  —  peripheral base address
// -----------------------------------------------------------------------
#define PERIPHERAL_BASE     0x3F000000UL
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000UL)

// -----------------------------------------------------------------------
// GPIO registers  (all 32-bit, word-aligned)
//
// Each GPFSEL register holds function-select bits for 10 GPIO pins.
// Three bits per pin:
//   000 = Input   001 = Output   1xx = Alternate functions 0-5
//
// GPSET / GPCLR / GPLEV are bitmaps; bit N corresponds to GPIO N.
// -----------------------------------------------------------------------
#define GPFSEL0     (*(volatile uint32_t *)(GPIO_BASE + 0x00))  // Pins  0-9
#define GPFSEL1     (*(volatile uint32_t *)(GPIO_BASE + 0x04))  // Pins 10-19
#define GPFSEL2     (*(volatile uint32_t *)(GPIO_BASE + 0x08))  // Pins 20-29
#define GPFSEL3     (*(volatile uint32_t *)(GPIO_BASE + 0x0C))  // Pins 30-39
#define GPFSEL4     (*(volatile uint32_t *)(GPIO_BASE + 0x10))  // Pins 40-49
#define GPFSEL5     (*(volatile uint32_t *)(GPIO_BASE + 0x14))  // Pins 50-53

#define GPSET0      (*(volatile uint32_t *)(GPIO_BASE + 0x1C))  // Set   pins  0-31
#define GPSET1      (*(volatile uint32_t *)(GPIO_BASE + 0x20))  // Set   pins 32-53

#define GPCLR0      (*(volatile uint32_t *)(GPIO_BASE + 0x28))  // Clear pins  0-31
#define GPCLR1      (*(volatile uint32_t *)(GPIO_BASE + 0x2C))  // Clear pins 32-53

#define GPLEV0      (*(volatile uint32_t *)(GPIO_BASE + 0x34))  // Level pins  0-31
#define GPLEV1      (*(volatile uint32_t *)(GPIO_BASE + 0x38))  // Level pins 32-53

// -----------------------------------------------------------------------
// Function-select values (3 bits)
// -----------------------------------------------------------------------
#define GPIO_FUNC_INPUT     0x0
#define GPIO_FUNC_OUTPUT    0x1
#define GPIO_FUNC_ALT0      0x4
#define GPIO_FUNC_ALT1      0x5
#define GPIO_FUNC_ALT2      0x6
#define GPIO_FUNC_ALT3      0x7
#define GPIO_FUNC_ALT4      0x3
#define GPIO_FUNC_ALT5      0x2

// -----------------------------------------------------------------------
// Board-specific pin assignments
// -----------------------------------------------------------------------
// Green ACT LED is wired to GPIO 29 on the Raspberry Pi Zero 2W
#define ACT_LED_GPIO        29u

// -----------------------------------------------------------------------
// Driver API
// -----------------------------------------------------------------------
void         gpio_set_function(unsigned int pin, unsigned int func);
void         gpio_set(unsigned int pin);
void         gpio_clear(unsigned int pin);
unsigned int gpio_read(unsigned int pin);

#endif /* GPIO_H */
