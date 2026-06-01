#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

// -----------------------------------------------------------------------
// SSD1306 I2C address (SA0 pin tied LOW → 0x3C, HIGH → 0x3D)
// -----------------------------------------------------------------------
#define SSD1306_ADDR        0x3C

// Display dimensions
#define SSD1306_WIDTH       128u
#define SSD1306_HEIGHT      64u
#define SSD1306_PAGES       (SSD1306_HEIGHT / 8u)  // 8

// I2C control bytes
#define SSD1306_CMD_STREAM  0x00  // All following bytes are commands
#define SSD1306_DATA_STREAM 0x40  // All following bytes are data

// -----------------------------------------------------------------------
// API
// -----------------------------------------------------------------------
void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_draw_string(uint8_t col, uint8_t page, const char *str);

#endif // SSD1306_H
