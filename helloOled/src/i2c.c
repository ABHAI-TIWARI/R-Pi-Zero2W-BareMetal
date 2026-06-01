#include "i2c.h"
#include "gpio.h"

// -----------------------------------------------------------------------
// Software (bit-bang) I2C
//
// GPIO 2 = SDA (physical pin 3)
// GPIO 3 = SCL (physical pin 5)
//
// Open-drain emulation:
//   HIGH → configure pin as INPUT  (external/internal pull-up holds line HIGH)
//   LOW  → configure pin as OUTPUT with value 0 (pin drives LOW)
//
// GPIO 2 and 3 have internal pull-ups enabled by default on BCM2837.
// -----------------------------------------------------------------------

// Direct GPFSEL0 access (covers GPIO 0-9)
#define GPFSEL0_REG  (*(volatile uint32_t *)(GPIO_BASE + 0x00))
#define GPSET0_REG   (*(volatile uint32_t *)(GPIO_BASE + 0x1C))
#define GPCLR0_REG   (*(volatile uint32_t *)(GPIO_BASE + 0x28))

static void pin_output_low(unsigned int pin)
{
    unsigned int shift = (pin % 10u) * 3u;
    GPCLR0_REG   = (1u << pin);                                          // value = 0
    GPFSEL0_REG  = (GPFSEL0_REG & ~(7u << shift)) | (1u << shift);      // func = OUTPUT
}

static void pin_input(unsigned int pin)
{
    unsigned int shift = (pin % 10u) * 3u;
    GPFSEL0_REG &= ~(7u << shift);                                       // func = INPUT
}

// I2C timing: each half-period ~5 µs  →  ~100 kHz
// At 600 MHz, 750 iterations × ~4 cycles ≈ 5 µs
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
    SDA_LOW();  i2c_delay();   // SDA falls while SCL is HIGH = START condition
    SCL_LOW();  i2c_delay();
}

static void i2c_stop(void)
{
    SDA_LOW();  i2c_delay();
    SCL_HIGH(); i2c_delay();
    SDA_HIGH(); i2c_delay();   // SDA rises while SCL is HIGH = STOP condition
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

    // ACK bit: release SDA so slave can pull it LOW
    SDA_HIGH();             // release (INPUT — pull-up holds HIGH, slave can drive LOW)
    i2c_delay();
    SCL_HIGH(); i2c_delay(); // clock the ACK bit (we don't read it — assume ACK)
    SCL_LOW();  i2c_delay();
    // SDA stays as INPUT; next byte starts by driving it
}

// -----------------------------------------------------------------------
// i2c_init — set SDA and SCL idle HIGH
// -----------------------------------------------------------------------
void i2c_init(void)
{
    SDA_HIGH();
    SCL_HIGH();
    i2c_delay();
    i2c_delay();
}

// -----------------------------------------------------------------------
// i2c_write — send len bytes to 7-bit slave address addr
// -----------------------------------------------------------------------
int i2c_write(uint8_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t i;

    i2c_start();
    i2c_write_byte((uint8_t)((addr << 1u) | 0u));  // 7-bit addr + write bit (0)
    for (i = 0u; i < len; i++) {
        i2c_write_byte(data[i]);
    }
    i2c_stop();

    return 0;
}
