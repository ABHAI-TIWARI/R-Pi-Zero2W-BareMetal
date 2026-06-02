#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_ADDR        0x3C
#define SSD1306_WIDTH       128u
#define SSD1306_HEIGHT      64u
#define SSD1306_PAGES       8u

#define SSD1306_CMD_STREAM  0x00
#define SSD1306_DATA_STREAM 0x40

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_draw_string(uint8_t col, uint8_t page, const char *str);

#endif // SSD1306_H
