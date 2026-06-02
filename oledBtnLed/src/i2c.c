#include "i2c.h"
#include "gpio.h"

// -----------------------------------------------------------------------
// Software (bit-bang) I2C
//
// GPIO 2 = SDA (physical pin 3)
// GPIO 3 = SCL (physical pin 5)
//
// Open-drain emulation:
//   HIGH → configure pin as INPUT  (pull-up holds line HIGH)
//   LOW  → configure pin as OUTPUT, drive LOW
//
// GPIO 2 and 3 have internal pull-ups on BCM2837 by default.
// -----------------------------------------------------------------------

#define GPFSEL0_REG  (*(volatile uint32_t *)(GPIO_BASE + 0x00))
#define GPSET0_REG   (*(volatile uint32_t *)(GPIO_BASE + 0x1C))
#define GPCLR0_REG   (*(volatile uint32_t *)(GPIO_BASE + 0x28))

static void pin_output_low(unsigned int pin)
{
    unsigned int shift = (pin % 10u) * 3u;
    GPCLR0_REG  = (1u << pin);
    GPFSEL0_REG = (GPFSEL0_REG & ~(7u << shift)) | (1u << shift);
}

static void pin_input(unsigned int pin)
{
    unsigned int shift = (pin % 10u) * 3u;
    GPFSEL0_REG &= ~(7u << shift);
}

// ~100 kHz: 750 iterations ≈ 5 µs half-period at 600 MHz
static void i2c_delay(void)
{
    volatile unsigned int i;
    for (i = 0u; i < 750u; i++) {}
}

#define SDA_HIGH()  pin_input(I2C_SDA_GPIO)
#define SDA_LOW()   pin_output_low(I2C_SDA_GPIO)
#define SCL_HIGH()  pin_input(I2C_SCL_GPIO)
#define SCL_LOW()   pin_output_low(I2C_SCL_GPIO)

static void i2c_start(void)
{
    SDA_HIGH(); i2c_delay();
    SCL_HIGH(); i2c_delay();
    SDA_LOW();  i2c_delay();    // SDA falls while SCL HIGH = START
    SCL_LOW();  i2c_delay();
}

static void i2c_stop(void)
{
    SDA_LOW();  i2c_delay();
    SCL_HIGH(); i2c_delay();
    SDA_HIGH(); i2c_delay();    // SDA rises while SCL HIGH = STOP
    i2c_delay();
}

static void i2c_write_byte(uint8_t byte)
{
    int i;
    for (i = 7; i >= 0; i--) {
        if (byte & (1u << i)) SDA_HIGH();
        else                   SDA_LOW();
        i2c_delay();
        SCL_HIGH(); i2c_delay();
        SCL_LOW();  i2c_delay();
    }
    // ACK clock — release SDA, clock one pulse, ignore result
    SDA_HIGH(); i2c_delay();
    SCL_HIGH(); i2c_delay();
    SCL_LOW();  i2c_delay();
}

void i2c_init(void)
{
    SDA_HIGH();
    SCL_HIGH();
    i2c_delay();
    i2c_delay();
}

int i2c_write(uint8_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t i;
    i2c_start();
    i2c_write_byte((uint8_t)((addr << 1u) | 0u));
    for (i = 0u; i < len; i++) {
        i2c_write_byte(data[i]);
    }
    i2c_stop();
    return 0;
}
