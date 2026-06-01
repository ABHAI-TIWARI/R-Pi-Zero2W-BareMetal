# helloOled — Bare-Metal SSD1306 OLED on Raspberry Pi Zero 2W

Displays **"Hello ABHAI.."** on a 128×64 SSD1306 OLED display using a fully
bare-metal, zero-OS implementation on the Raspberry Pi Zero 2W.

No Linux. No device drivers. No libraries. Every bit on the I2C bus is
toggled directly by the firmware.

---

## Hardware

| Component | Detail |
|---|---|
| Board | Raspberry Pi Zero 2W (BCM2710A1 / BCM2837, quad-core Cortex-A53) |
| Display | SSD1306 128×64 monochrome OLED, I2C interface |
| I2C address | `0x3C` (SA0 pin tied LOW) |
| SDA | GPIO 2 — Physical pin 3 |
| SCL | GPIO 3 — Physical pin 5 |
| VCC | 3.3 V — Physical pin 1 |
| GND | GND — Physical pin 6 |

### Wiring Diagram

```
RPi Zero 2W                  SSD1306 OLED
─────────────                ────────────
Pin 1  (3.3V) ──────────────► VCC
Pin 3  (GPIO2/SDA) ─────────► SDA
Pin 5  (GPIO3/SCL) ─────────► SCL
Pin 6  (GND)  ──────────────► GND
```

> GPIO 2 and GPIO 3 have internal pull-up resistors enabled by default on the
> BCM2837.  No external pull-up resistors are required.

---

## What It Does

1. Waits ~100 ms after boot for power rails to stabilise.
2. Initialises the software I2C driver (sets SDA and SCL idle HIGH).
3. Sends the full SSD1306 128×64 initialisation command sequence.
4. Clears the entire display RAM to black.
5. Draws **"Hello ABHAI.."** centred horizontally on page 3 (vertical middle of the screen).
6. Enters a low-power `wfe` (Wait For Event) idle loop forever.

---

## Project Structure

```
helloOled/
├── config.txt          Boot configuration for the Pi firmware
├── linker.ld           Linker script — places kernel at 0x8000
├── Makefile            Build system
└── src/
    ├── boot.S          AArch32 startup — parks secondary cores, zeroes BSS, calls main()
    ├── gpio.h          BCM2837 peripheral base addresses and GPIO register map
    ├── gpio.c          gpio_set_function() — configure a GPIO pin's alternate function
    ├── i2c.h           Software I2C API
    ├── i2c.c           Software bit-bang I2C driver
    ├── ssd1306.h       SSD1306 driver API and constants
    ├── ssd1306.c       SSD1306 driver + 5×8 ASCII font
    └── main.c          Entry point
```

---

## Source Files Explained

### `boot.S` — Startup

The BCM2837 boots all four Cortex-A53 cores simultaneously. Only core 0
should run the application; cores 1–3 must be parked.

```asm
mrc  p15, 0, r1, c0, c0, 5   // Read MPIDR (core affinity register)
and  r1, r1, #0x3             // Extract core ID (bits 1:0)
cmp  r1, #0
bne  .Lhalt                   // Cores 1/2/3 → infinite WFE loop
```

Core 0 then:
- Sets the stack pointer to `0x8000` (kernel load address — stack grows down from there).
- Zeroes the `.bss` section (uninitialised global variables) using `stmia`.
- Calls `main()`.

---

### `gpio.h` / `gpio.c` — GPIO Driver

The BCM2837 peripheral bus is mapped at ARM physical address `0x3F000000`.
The GPIO controller sits at `0x3F200000`.

Each GPIO pin's function is selected by 3 bits in one of the `GPFSELn`
registers (10 pins per register, 3 bits each).

```c
// gpio_set_function — select the function of a GPIO pin
// func values: 0=INPUT, 1=OUTPUT, 4=ALT0, 5=ALT1, ...
void gpio_set_function(unsigned int pin, unsigned int func);
```

In this project `gpio_set_function` is used internally by the I2C driver to
dynamically switch GPIO 2 and GPIO 3 between INPUT and OUTPUT for open-drain
emulation.

---

### `i2c.h` / `i2c.c` — Software Bit-Bang I2C

Rather than using the BCM2837's built-in BSC (Broadcom Serial Controller)
hardware peripheral, this project implements I2C entirely in software.  This
approach directly controls the timing of every bit and avoids any hardware
register sequencing issues.

#### Why software I2C?

The BCM2837 BSC peripheral requires careful sequencing of DLEN, FIFO, and
control registers with exact timing.  Getting it wrong produces no output on
the bus at all — with no error feedback from bare metal.  Software I2C is
simpler, more debuggable, and fully deterministic.

#### Open-drain emulation

I2C is an open-drain bus — no device ever drives a line HIGH; it only pulls
it LOW or releases it.  The BCM2837 does not have a hardware open-drain output
mode, so it is emulated in software:

| Desired line state | GPIO configuration |
|---|---|
| HIGH | Pin set to **INPUT** — the internal pull-up resistor holds the line HIGH |
| LOW | Pin set to **OUTPUT** with value 0 — pin actively drives the line LOW |

```c
#define SDA_HIGH()  pin_input(I2C_SDA_GPIO)       // release → pull-up holds HIGH
#define SDA_LOW()   pin_output_low(I2C_SDA_GPIO)  // drive LOW
#define SCL_HIGH()  pin_input(I2C_SCL_GPIO)
#define SCL_LOW()   pin_output_low(I2C_SCL_GPIO)
```

#### I2C timing

Each half clock period is produced by a busy-wait loop of 750 iterations
which takes approximately 5 µs at 600 MHz, giving a clock frequency of
roughly **100 kHz** — the standard I2C speed.

#### I2C transaction structure

```
START  →  [addr<<1 | 0 (write)]  →  [byte 0]  →  [byte 1]  →  ...  →  STOP
```

- **START condition**: SDA goes LOW while SCL is HIGH.
- **STOP condition**: SDA goes HIGH while SCL is HIGH.
- **Data bits**: each bit is placed on SDA before SCL rises; sampled by the
  slave on the SCL rising edge.
- **ACK bit**: after each byte the master releases SDA (INPUT) and clocks one
  SCL pulse. The slave pulls SDA LOW to acknowledge. This firmware generates
  the ACK clock but does not read back the value (assumes ACK).

#### API

```c
void i2c_init(void);
// Sets SDA and SCL idle HIGH. Must be called before any transfers.

int i2c_write(uint8_t addr, const uint8_t *data, uint32_t len);
// Sends `len` bytes from `data` to the 7-bit slave address `addr`.
// Returns 0 always (ACK not checked in this implementation).
```

---

### `ssd1306.h` / `ssd1306.c` — OLED Display Driver

The SSD1306 is a single-chip OLED driver for 128×64 dot-matrix displays.
It communicates over I2C and stores pixel data in its own internal GDDRAM
(Graphic Display Data RAM).

#### I2C protocol

Every I2C transaction to the SSD1306 starts with a **control byte**:

| Control byte | Meaning |
|---|---|
| `0x00` | All following bytes in this transaction are **commands** |
| `0x40` | All following bytes in this transaction are **pixel data** |

#### Display memory layout

The display is divided into 8 **pages** (rows of 8 pixels each).
Each page contains 128 **columns** (one byte per column, 8 vertical pixels).

```
Page 0  → rows 0-7
Page 1  → rows 8-15
Page 2  → rows 16-23
Page 3  → rows 24-31   ← "Hello ABHAI.." is drawn here (vertical centre)
Page 4  → rows 32-39
Page 5  → rows 40-47
Page 6  → rows 48-55
Page 7  → rows 56-63
```

In each column byte, **bit 0 is the top pixel** of that page row.

**Horizontal addressing mode** (`0x20, 0x00`) is used: after writing a column
the GDDRAM pointer advances automatically to the next column, wrapping to the
next page at column 127.

#### `ssd1306_init()`

Sends a 27-byte command sequence in a single I2C transaction that configures:

| Command | Purpose |
|---|---|
| `0xAE` | Display OFF (before configuration) |
| `0xD5 0x80` | Display clock oscillator frequency |
| `0xA8 0x3F` | Multiplex ratio: 64 rows |
| `0xD3 0x00` | Display offset: none |
| `0x40` | Display start line: 0 |
| `0x8D 0x14` | Enable charge pump (powers the OLED panel) |
| `0x20 0x00` | Horizontal memory addressing mode |
| `0xA1` | Segment re-map (horizontal flip for natural orientation) |
| `0xC8` | COM scan direction remapped (vertical flip) |
| `0xDA 0x12` | COM pin configuration for 128×64 |
| `0x81 0xCF` | Contrast: high (0xCF) |
| `0xD9 0xF1` | Pre-charge period |
| `0xDB 0x40` | VCOMH deselect voltage |
| `0xA4` | Display content from RAM |
| `0xA6` | Normal (non-inverted) display |
| `0xAF` | Display ON |

#### `ssd1306_clear()`

Sets the column range (0–127) and page range (0–7) then writes 1024 zero bytes
as pixel data, turning every pixel off.  The data is sent in 32 chunks of 32
bytes each to keep stack usage small (no large stack buffers).

#### `ssd1306_draw_string(col, page, str)`

Renders a null-terminated ASCII string starting at the given column and page.

The font is a standard 5×7 LCD bitmap font stored as a `const` array of 95
entries (characters `0x20` space through `0x7E` tilde), one entry per
character, 5 bytes per entry.  Each byte is one vertical column of pixels
(bit 0 = top).

For each character a 7-byte I2C data transaction is sent:

```
[ 0x40,  col0, col1, col2, col3, col4,  0x00 ]
  ctrl   ←────── 5 font bytes ──────→  spacing
```

The trailing `0x00` byte provides a 1-pixel gap between characters, giving an
effective character width of **6 pixels**.

`"Hello ABHAI.."` is 13 characters × 6 pixels = **78 pixels** wide.
Starting column = `(128 − 78) / 2 = 25` → horizontally centred.

---

### `main.c` — Entry Point

```c
void main(void)
{
    delay(300000u);           // ~100 ms power-on stabilisation
    i2c_init();               // set SDA/SCL idle HIGH
    ssd1306_init();           // send display configuration commands
    ssd1306_clear();          // blank the screen
    ssd1306_draw_string(25u, 3u, "Hello ABHAI..");  // draw centred text
    while (1) { __asm__ volatile("wfe"); }           // low-power idle
}
```

The `wfe` (Wait For Event) instruction halts the core until an interrupt or
event occurs, consuming minimal power while keeping the display contents
intact (OLED display RAM is in the SSD1306 chip, not the CPU).

---

### `linker.ld` — Linker Script

Places the kernel at address `0x8000`, which is where the Raspberry Pi GPU
firmware (`start.elf`) jumps to after loading `kernel.img` from the SD card.

Section order: `.text.boot` first (contains `_start`), then `.text`, `.rodata`,
`.data`, `.bss`.  The symbols `__bss_start` and `__bss_end` are exported for
`boot.S` to use when zeroing uninitialised memory.

---

### `config.txt` — Boot Configuration

```ini
arm_64bit=0       # Run in AArch32 (32-bit ARM) mode
kernel=kernel.img # Load this file as the kernel
dtparam=audio=off # Disable audio (saves power, no interference)
start_x=0         # Disable camera firmware
enable_uart=0     # Disable UART (not used)
```

`arm_64bit=0` is essential — the code is compiled for AArch32
(`-mcpu=cortex-a53 -mfloat-abi=hard`). Running in 64-bit mode would cause
an immediate crash.

---

## Build

### Prerequisites

| Tool | Version used |
|---|---|
| `arm-none-eabi-gcc` | 10.3.1 (GNU Arm Embedded Toolchain 2021.10) |
| `arm-none-eabi-ld` | bundled with above |
| GNU Make | 4.4.1 |

### Build command

```powershell
# From the repository root
make helloOled
```

Or from inside the `helloOled/` directory:

```powershell
make
```

Output files are placed in `helloOled/img/`:

| File | Purpose |
|---|---|
| `kernel.elf` | ELF with debug symbols |
| `kernel.img` | Raw binary loaded by the Pi bootloader |
| `config.txt` | Boot configuration (copied from project root) |

### Expected output

```
arm-none-eabi-gcc ... -c -o obj/main.o src/main.c
...
arm-none-eabi-size img/kernel.elf
   text    data     bss     dec     hex filename
   2351       0       0    2351     92f img/kernel.elf
```

Total binary size is approximately **2.3 KB**.

---

## Flashing to SD Card

1. Format an SD card as **FAT32**.
2. Copy the Raspberry Pi firmware files onto it:
   - `bootcode.bin`
   - `start.elf`
   - `fixup.dat`
3. Copy the build output:

```powershell
Copy-Item "helloOled\img\kernel.img" -Destination "F:\" -Force
Copy-Item "helloOled\img\config.txt" -Destination "F:\" -Force
```

4. Eject the SD card, insert into the Pi, and power on.

---

## How the Boot Sequence Works

```
Power on
   │
   ▼
GPU (VideoCore IV) starts first
   │  reads bootcode.bin from SD card
   ▼
start.elf runs
   │  loads kernel.img to RAM address 0x8000
   │  reads config.txt (arm_64bit=0 → AArch32 mode)
   ▼
CPU cores released, all jump to 0x8000 (_start in boot.S)
   │
   ├─ Cores 1/2/3 → MPIDR check → WFE loop (parked)
   │
   └─ Core 0 continues:
         │  SP = 0x8000 (stack grows down)
         │  BSS zeroed
         ▼
       main() called
         │  delay 100 ms
         │  i2c_init()
         │  ssd1306_init()
         │  ssd1306_clear()
         │  ssd1306_draw_string(25, 3, "Hello ABHAI..")
         ▼
       WFE idle loop
```

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Display stays blank | Wrong I2C address | Change `SSD1306_ADDR` to `0x3D` in `ssd1306.h` if the module's SA0 pin is HIGH |
| Display stays blank | Wrong display size | If using 128×32, change `0xA8 0x3F → 0xA8 0x1F` and `0xDA 0x12 → 0xDA 0x02` in `ssd1306_init()` |
| Display shows noise / garbage | Clock too fast for long wires | Increase `i2c_delay` loop count (try 1500) in `i2c.c` |
| Display shows noise / garbage | Clock too slow | Decrease `i2c_delay` loop count (try 200) |
| Pi does not boot | Wrong `arm_64bit` setting | Ensure `arm_64bit=0` in `config.txt` |
| No power to display | VCC wiring | Confirm VCC → Pin 1 (3.3 V), not Pin 2/4 (5 V — may damage the module) |
