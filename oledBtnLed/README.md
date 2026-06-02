# oledBtnLed — Bare-Metal RGB LED Controller with OLED Display

Controls an RGB LED with two tactile switches and shows live status on a
128×64 SSD1306 OLED display.  Fully bare-metal on the Raspberry Pi Zero 2W —
no OS, no libraries, no drivers.

---

## What It Does

| Action | Result |
|---|---|
| Press **SW1** (LED off) | LED turns **ON** at the current colour |
| Press **SW1** (LED on) | LED turns **OFF** |
| Press **SW2** (LED on) | Cycles to the next colour |
| Press **SW2** (LED off) | Ignored |

The OLED display updates instantly on every button press:

```
┌─────────────────────────────┐
│                             │  Page 0
│      LED Controller         │
│                             │  Page 1-2
│         Status: ON          │  Page 3
│                             │  Page 4
│       Color: Magenta        │  Page 5
│                             │  Page 6-7
└─────────────────────────────┘
```

When the LED is off, the color row is blank.

### Colour Sequence (SW2 cycles forward)

| Step | Color | R | G | B |
|---|---|---|---|---|
| 0 | Red | ● | ○ | ○ |
| 1 | Green | ○ | ● | ○ |
| 2 | Blue | ○ | ○ | ● |
| 3 | Yellow | ● | ● | ○ |
| 4 | Cyan | ○ | ● | ● |
| 5 | Magenta | ● | ○ | ● |
| 6 | White | ● | ● | ● |

Wraps back to Red after White.

---

## Hardware

### Components

| Component | Detail |
|---|---|
| Board | Raspberry Pi Zero 2W (BCM2710A1 / BCM2837, Cortex-A53) |
| LED | RGB LED — common cathode, one 330 Ω resistor per channel |
| Switches | 2× tactile switches with 4.7 kΩ pull-up resistors to 3.3 V |
| Display | SSD1306 128×64 monochrome OLED, I2C |

### Pin Connections

| Signal | GPIO | Physical Pin | Notes |
|---|---|---|---|
| RGB Red | GPIO 22 | Pin 15 | 330 Ω to LED R anode |
| RGB Green | GPIO 27 | Pin 13 | 330 Ω to LED G anode |
| RGB Blue | GPIO 17 | Pin 11 | 330 Ω to LED B anode |
| LED Cathode | — | Pin 6 (GND) | Common cathode |
| SW1 (on/off) | GPIO 16 | Pin 36 | 4.7 kΩ pull-up to 3.3 V |
| SW2 (color) | GPIO 20 | Pin 38 | 4.7 kΩ pull-up to 3.3 V |
| OLED SDA | GPIO 2 | Pin 3 | Internal pull-up on BCM2837 |
| OLED SCL | GPIO 3 | Pin 5 | Internal pull-up on BCM2837 |
| OLED VCC | — | Pin 1 (3.3 V) | |
| OLED GND | — | Pin 6 (GND) | |

### Wiring Diagram

```
RPi Zero 2W                     RGB LED (common cathode)
───────────────                 ────────────────────────
Pin 15 (GPIO22) ──[330Ω]──────► R anode
Pin 13 (GPIO27) ──[330Ω]──────► G anode
Pin 11 (GPIO17) ──[330Ω]──────► B anode
Pin 6  (GND)   ───────────────► Cathode

                                Tactile Switches
                                ────────────────
Pin 1  (3.3V)  ──[4.7kΩ]──┬──► SW1 pin 1
                           └──► SW1 pin 2 ──► Pin 36 (GPIO16)

Pin 1  (3.3V)  ──[4.7kΩ]──┬──► SW2 pin 1
                           └──► SW2 pin 2 ──► Pin 38 (GPIO20)

                                SSD1306 OLED
                                ────────────
Pin 1  (3.3V)  ───────────────► VCC
Pin 3  (GPIO2) ───────────────► SDA
Pin 5  (GPIO3) ───────────────► SCL
Pin 6  (GND)   ───────────────► GND
```

> **Switch wiring:** one side of each switch goes to the GPIO pin, the other
> side to GND.  The pull-up resistor connects the GPIO pin to 3.3 V.
> Pressing the switch pulls the GPIO pin LOW (active-LOW logic).

---

## Project Structure

```
oledBtnLed/
├── config.txt          Boot configuration
├── linker.ld           Places kernel at 0x8000
├── Makefile            Build system
└── src/
    ├── boot.S          AArch32 startup — parks cores 1-3, zeroes BSS, calls main()
    ├── gpio.h          BCM2837 register map, pin assignments for LED, switches, I2C
    ├── gpio.c          gpio_set_function / gpio_set / gpio_clear / gpio_read
    ├── i2c.h           Software I2C API
    ├── i2c.c           Software bit-bang I2C driver (GPIO 2/3, ~100 kHz)
    ├── ssd1306.h       SSD1306 driver API
    ├── ssd1306.c       SSD1306 driver + full 5×8 ASCII font
    └── main.c          Application logic
```

---

## Source Files Explained

### `gpio.h` — Pin Assignments

All hardware pin assignments are centralised here.  The three defines that
map GPIO numbers to RGB channels are the most hardware-specific part of the
project — these were tuned to match the physical wiring:

```c
#define RGB_RED_GPIO    22u   // Physical pin 15
#define RGB_GREEN_GPIO  27u   // Physical pin 13
#define RGB_BLUE_GPIO   17u   // Physical pin 11

#define SW1_GPIO        16u   // Physical pin 36 — toggle on/off
#define SW2_GPIO        20u   // Physical pin 38 — cycle colour
```

### `gpio.c` — GPIO Driver

Four functions cover everything needed:

| Function | Purpose |
|---|---|
| `gpio_set_function(pin, func)` | Set a pin to INPUT, OUTPUT, or ALT mode |
| `gpio_set(pin)` | Drive pin HIGH |
| `gpio_clear(pin)` | Drive pin LOW |
| `gpio_read(pin)` | Read current logic level (1 or 0) |

Registers are accessed directly via pointer casts to physical addresses.
`gpio_set_function` reads the current `GPFSELn` value, masks the 3-bit field
for the target pin, writes the new function code, and writes back — a safe
read-modify-write that does not disturb other pins.

### `i2c.c` — Software Bit-Bang I2C

The BCM2837 has a hardware I2C peripheral (BSC), but this project uses a
software implementation instead for reliability and simplicity.

**Open-drain emulation:**  I2C lines must never be driven HIGH — only released
(pulled HIGH by a resistor) or driven LOW.  BCM2837 has no hardware open-drain
output mode, so it is emulated:

```
Line HIGH  →  configure pin as INPUT   (built-in pull-up holds it HIGH)
Line LOW   →  configure pin as OUTPUT, drive 0
```

GPIO 2 and GPIO 3 have internal pull-ups enabled by default on BCM2837, so
no external pull-up resistors are needed for the I2C lines.

The clock runs at approximately **100 kHz** — each half-period is a busy-wait
loop of 750 iterations (~5 µs at 600 MHz).

### `ssd1306.c` — OLED Driver

The display is controlled through three functions:

| Function | What it does |
|---|---|
| `ssd1306_init()` | Sends 26-byte init sequence; turns display ON |
| `ssd1306_clear()` | Writes 1024 zero bytes to GDDRAM (blanks screen) |
| `ssd1306_draw_string(col, page, str)` | Renders text using a built-in 5×8 font |

The display memory is organised as 8 **pages** (rows of 8 pixels) × 128
**columns**.  Horizontal addressing mode is used, so the write pointer
advances automatically after each byte.

Each character uses 5 font bytes + 1 spacing byte = **6 pixels wide**.
All strings on the same display row are padded to the same character count so
that shorter text overwrites longer text without a full screen clear.

### `main.c` — Application Logic

**Colour table and display strings** are both indexed by the same
`colour_idx` integer, ensuring the LED and OLED are always in sync:

```c
colour_table[colour_idx]  →  {R, G, B} bits  →  gpio_set/gpio_clear
colour_str[colour_idx]    →  "Color: ..."     →  ssd1306_draw_string
```

**Debounce strategy** (applies to both buttons identically):
1. Detect falling edge — `gpio_read(pin) == 0`
2. Wait 375,000 NOP ticks (~2.5 ms) for bounce to settle
3. Confirm pin is still LOW (valid press)
4. Execute action
5. Spin until pin returns HIGH (release)
6. Wait again for release bounce

**`led_apply(on, idx)`** — applies LED state in one place.  If `on = 0` all
three channels are cleared regardless of `idx`, so the colour index is
preserved while the LED is off and restored correctly when turned back on.

**`oled_update(on, idx)`** — redraws only the two dynamic rows (Status and
Color).  The header "LED Controller" is drawn once at startup and never
touched again, saving I2C transactions on every button press.

---

## Build

### Prerequisites

| Tool | Version |
|---|---|
| `arm-none-eabi-gcc` | 10.3.1 |
| GNU Make | 4.4.1 |

### Commands

```powershell
# From the repository root
make oledBtnLed

# Or from inside the project folder
cd oledBtnLed
make
```

Output lands in `oledBtnLed/img/`:

```
img/kernel.img   ← copy this to the SD card
img/config.txt   ← copy this to the SD card
img/kernel.elf   ← ELF with debug symbols (not needed on SD card)
```

Expected size: ~3.4 KB.

---

## Flashing to SD Card

1. Format SD card as **FAT32**.
2. Copy the Raspberry Pi firmware onto it: `bootcode.bin`, `start.elf`, `fixup.dat`.
3. Copy the build output:

```powershell
Copy-Item "oledBtnLed\img\kernel.img" -Destination "F:\" -Force
Copy-Item "oledBtnLed\img\config.txt" -Destination "F:\" -Force
```

4. Eject, insert into the Pi, power on.

---

## Boot Sequence

```
Power on → GPU loads bootcode.bin → start.elf loads kernel.img to 0x8000
    │
    ▼
All 4 Cortex-A53 cores jump to _start (boot.S)
    │
    ├─ Cores 1/2/3  →  MPIDR check → WFE loop (parked)
    └─ Core 0 continues:
          SP = 0x8000  │  BSS zeroed  │  main() called
              │
              ├─ GPIO setup (RGB outputs, SW inputs)
              ├─ LED off
              ├─ delay 100 ms  (power rail stabilisation)
              ├─ i2c_init()
              ├─ ssd1306_init()
              ├─ ssd1306_clear()
              ├─ Draw "LED Controller"  (header — once only)
              ├─ Draw "Status: OFF"
              │
              └─ while(1)
                    poll SW1 → toggle LED + update OLED Status + Color rows
                    poll SW2 → cycle colour + update OLED Color row
```

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| OLED shows nothing | Wrong I2C address | Change `SSD1306_ADDR` to `0x3D` in `ssd1306.h` |
| OLED shows nothing | I2C too fast for cable length | Increase delay loop count in `i2c.c` (try 1500) |
| LED wrong color | RGB channels swapped | Swap the `RGB_*_GPIO` defines in `gpio.h` to match physical wiring |
| LED always on / always off | Common anode LED | Invert `gpio_set` / `gpio_clear` calls in `led_apply()` |
| Button needs multiple presses | Bounce too wide | Increase `DEBOUNCE_TICKS` in `main.c` (try 750000) |
| Button feels sluggish | Debounce too long | Decrease `DEBOUNCE_TICKS` (try 200000) |
| Pi does not boot | Wrong CPU mode | Ensure `arm_64bit=0` in `config.txt` |
