#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// Software (bit-bang) I2C — GPIO 2 = SDA, GPIO 3 = SCL
// Avoids all BSC peripheral register complexity.

void i2c_init(void);
int  i2c_write(uint8_t addr, const uint8_t *data, uint32_t len);

#endif // I2C_H
