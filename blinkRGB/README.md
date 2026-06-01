# blinkRGB — RGB LED Colour Cycle

Cycles an external common-cathode RGB LED through 7 colours at ~33 ms per colour.  
No OS, no HAL, no standard library — direct register access on bare metal.

---

## Hardware

| Item | Detail |
|------|--------|
| Board | Raspberry Pi Zero 2W |
| SoC | BCM2710A1 (quad-core Cortex-A53, AArch32) |
| LED | Common-cathode RGB LED (external) |
| Red channel | GPIO 17 — Physical pin 11 |
| Green channel | GPIO 27 — Physical pin 13 |
| Blue channel | GPIO 22 — Physical pin 15 |
| GND | Physical pin 9, 14, 20, 25, 30, 34, or 39 |

### Wiring Diagram

```
RPi Zero 2W 40-pin header
                                         RGB LED
  Physical pin 11  (GPIO 17) ── 330Ω ──  R (red anode)   ┐
  Physical pin 13  (GPIO 27) ── 330Ω ──  G (green anode)  ├─ Common cathode → GND
  Physical pin 15  (GPIO 22) ── 330Ω ──  B (blue anode)   ┘
  Physical pin 14  (GND)     ────────────────────────────────────────────────────
```

> **Important:** Each channel needs a **330Ω resistor** in series.  
> RPi GPIO pins source/sink a maximum of 16 mA. Without a resistor you risk  
> permanently damaging the GPIO pin or the SoC.

### Common Cathode vs Common Anode

| Type | Common pin | GPIO HIGH | GPIO LOW |
|------|-----------|-----------|----------|
| **Common Cathode** ← this project | GND | LED ON ✅ | LED OFF |
| Common Anode | 3.3V | LED OFF | LED ON |

---

## Colour Sequence

The LED cycles through all 7 primary/secondary colours, one per step:

| Step | Colour | R (GPIO 17) | G (GPIO 27) | B (GPIO 22) |
|------|--------|:-----------:|:-----------:|:-----------:|
| 1 | Red | HIGH | LOW | LOW |
| 2 | Green | LOW | HIGH | LOW |
| 3 | Blue | LOW | LOW | HIGH |
| 4 | Yellow | HIGH | HIGH | LOW |
| 5 | Cyan | LOW | HIGH | HIGH |
| 6 | Magenta | HIGH | LOW | HIGH |
| 7 | White | HIGH | HIGH | HIGH |

Each colour holds for **5 000 000 NOP ticks ≈ 33 ms** at 600 MHz boot clock.  
To slow it down to ~1 second per colour, change the delay value to `150000000u`.

---

## Project Structure

```
blinkRGB/
├── src/
│   ├── boot.S      — AArch32 startup: park cores 1-3, zero BSS, call main()
│   ├── main.c      — RGB colour-cycle loop
│   ├── gpio.c      — GPIO driver (GPFSEL, GPSET, GPCLR, GPLEV)
│   └── gpio.h      — BCM2710A1 register map, API, and pin assignments
├── obj/            — Build artefacts (generated): .o files, kernel.elf
├── img/            — SD card output (generated): kernel.img, config.txt
├── linker.ld       — Memory layout: kernel at 0x8000
├── Makefile
└── README.md
```

### Key files explained

#### `boot.S`
ARM assembly startup code. The RPi firmware loads `kernel.img` to `0x8000` and  
releases all four Cortex-A53 cores simultaneously. `boot.S` does three things:
1. Reads `MPIDR` to get the CPU ID — cores 1, 2, 3 are parked in a `wfe` loop.
2. Sets `SP = 0x8000` (stack grows downward below the image).
3. Zeroes the `.bss` section (required by the C standard), then calls `main()`.

#### `gpio.h` / `gpio.c`
Direct memory-mapped GPIO driver for the BCM2710A1:
- `gpio_set_function(pin, func)` — writes to the correct `GPFSELn` register to  
  configure a pin as input, output, or an alternate function.
- `gpio_set(pin)` — writes bit N of `GPSET0/1` to drive the pin HIGH.
- `gpio_clear(pin)` — writes bit N of `GPCLR0/1` to drive the pin LOW.
- `gpio_read(pin)` — reads bit N of `GPLEV0/1`.

Register addresses (ARM physical — not the GPU bus address in the datasheet):

| Register | Address | Purpose |
|----------|---------|---------|
| GPFSEL0–5 | `0x3F200000` + offset | Function select (3 bits per pin) |
| GPSET0 | `0x3F20001C` | Set pins 0–31 HIGH |
| GPCLR0 | `0x3F200028` | Set pins 0–31 LOW |
| GPLEV0 | `0x3F200034` | Read level of pins 0–31 |

#### `linker.ld`
Places `.text.boot` (containing `_start`) exactly at `0x8000`.  
Exports `__bss_start` and `__bss_end` so `boot.S` can zero the BSS.

#### `config.txt`
Tells the RPi GPU firmware how to boot:
- `arm_64bit=0` — start in 32-bit AArch32 mode (matches `arm-none-eabi` toolchain).
- `kernel=kernel.img` — load our flat binary instead of the default Linux kernel.
- `enable_uart=0` — keeps GPIO 14/15 free for a future UART driver.

---

## Build

### Prerequisites

| Tool | Version | Location |
|------|---------|----------|
| `arm-none-eabi-gcc` | 10.3.1 | `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin` |
| `make` | GNU Make 4.4.1 | `C:\NXP\S32DS.3.5\S32DS\build_tools\msys32\usr\bin\make.exe` |

### Build commands

From the **repo root**:

```powershell
# Refresh PATH (needed once per new PowerShell session)
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("PATH","User")

# Build
make
```

Or from inside this folder:

```powershell
cd blinkRGB
make
```

### Build output

```
arm-none-eabi-gcc ... -c -o obj/main.o  src/main.c
arm-none-eabi-gcc ... -c -o obj/gpio.o  src/gpio.c
arm-none-eabi-gcc ... -c -o obj/boot.o  src/boot.S
arm-none-eabi-ld  ... -o img/kernel.elf obj/main.o obj/gpio.o obj/boot.o
arm-none-eabi-size img/kernel.elf

   text    data     bss     dec     hex filename
    824       0       0     824     338 img/kernel.elf

  Build complete: img/kernel.img
  SD card files ready in img/
```

| Directory | Contents |
|-----------|----------|
| `obj/` | `main.o`, `gpio.o`, `boot.o` — compiled object files |
| `obj/` | `kernel.elf` — linked ELF with debug symbols |
| `img/` | `kernel.img` — flat binary (824 bytes) copied to SD card |
| `img/` | `config.txt` — copied from project root to SD card |

### Other targets

```powershell
make dump    # disassemble kernel.elf → obj/kernel.dump (useful for debugging)
make clean   # delete obj/ and img/ entirely
```

---

## Flash to SD Card

### SD card requirements
- Format: **FAT32** (use [guiformat](http://ridgecrop.co.uk/guiformat.exe) on Windows for cards > 32 GB — Windows cannot format large cards as FAT32 natively)
- Firmware files from the [RPi firmware repo](https://github.com/raspberrypi/firmware/tree/master/boot)

### SD card root layout

```
F:\  (SD card)
├── bootcode.bin    ← GPU first-stage bootloader
├── start.elf       ← GPU firmware
├── fixup.dat       ← GPU memory split configuration
├── config.txt      ← from img/  (copied by make automatically)
└── kernel.img      ← from img/  (your bare-metal binary)
```

### Copy command (PowerShell)

```powershell
Copy-Item "blinkRGB\img\kernel.img", "blinkRGB\img\config.txt" F:\
```

---

## How It Works — Step by Step

```
Power on
  │
  ▼
GPU runs bootcode.bin
  │  reads config.txt → arm_64bit=0, kernel=kernel.img
  ▼
GPU loads start.elf, copies kernel.img → 0x8000
  │
  ▼
All 4 Cortex-A53 cores start at 0x8000  (boot.S _start)
  │
  ├─ Cores 1,2,3 → read MPIDR, see CPU ID ≠ 0 → wfe spin loop (parked)
  │
  └─ Core 0 → set SP = 0x8000
              zero BSS
              bl main()
                │
                ▼
              gpio_set_function(17, OUTPUT)   ← configure R pin
              gpio_set_function(27, OUTPUT)   ← configure G pin
              gpio_set_function(22, OUTPUT)   ← configure B pin
                │
                ▼
              infinite loop:
                Red      → delay → Green → delay → Blue  → delay →
                Yellow   → delay → Cyan  → delay → Magenta → delay →
                White    → delay → (repeat)
```

---

## Memory Map

| Region | Address |
|--------|---------|
| Kernel load / stack top | `0x0000_8000` |
| Peripheral base | `0x3F00_0000` |
| GPIO base | `0x3F20_0000` |
| GPFSEL2 (controls GPIO 20-29) | `0x3F20_0008` |
| GPFSEL1 (controls GPIO 10-19) | `0x3F20_0004` |
| GPSET0 | `0x3F20_001C` |
| GPCLR0 | `0x3F20_0028` |
