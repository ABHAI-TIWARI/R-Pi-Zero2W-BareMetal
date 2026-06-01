# Raspberry Pi Zero 2W — Bare Metal

A bare-metal **C + ARM Assembly** project for the Raspberry Pi Zero 2W.  
No OS, no HAL, no standard library — just your code running directly on the hardware.

Projects so far: ACT LED blink (GPIO 29) and RGB LED colour cycle (GPIO 17/27/22).

---

## Hardware

| Item | Detail |
|------|--------|
| Board | Raspberry Pi Zero 2W |
| SoC | Broadcom BCM2710A1 (quad-core ARM Cortex-A53) |
| Execution mode | AArch32 (32-bit ARM) |
| ACT LED | GPIO 29 |
| Peripheral base | `0x3F000000` |

---

## Prerequisites

| Tool | Purpose |
|------|---------|
| `arm-none-eabi-gcc` | Cross-compiler — [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| `make` | Build system |

### Installing the toolchain

**Windows** (via winget):
```powershell
winget install Arm.GnuArmEmbeddedToolchain
```
Then add the `bin/` folder to your `PATH`.

**Ubuntu / Debian**:
```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

**macOS** (via Homebrew):
```bash
brew install --cask gcc-arm-embedded
```

Verify:
```bash
arm-none-eabi-gcc --version
```

---

## Project Structure

```
.
├── blink/                      # Project 1 — On-board ACT LED blink (GPIO 29)
│   ├── src/
│   │   ├── boot.S              # Startup assembly — CPU init, BSS clear, stack setup
│   │   ├── main.c              # Blink loop — GPIO 29 on/off at ~1 Hz
│   │   ├── gpio.c              # GPIO driver implementation
│   │   └── gpio.h              # BCM2710A1 register map and API
│   ├── obj/                    # Build artefacts (generated) — .o files, kernel.elf
│   ├── img/                    # SD card output (generated) — kernel.img, config.txt
│   ├── linker.ld               # Memory layout (kernel at 0x8000)
│   ├── config.txt              # RPi firmware boot configuration
│   ├── Makefile
│   └── README.md
├── blinkRGB/                   # Project 2 — External RGB LED colour cycle
│   ├── src/
│   │   ├── boot.S              # Same startup code as blink/
│   │   ├── main.c              # 7-colour cycle: R→G→B→Yellow→Cyan→Magenta→White
│   │   ├── gpio.c              # GPIO driver implementation
│   │   └── gpio.h              # Register map + RGB pin assignments (GPIO 17/27/22)
│   ├── obj/                    # Build artefacts (generated)
│   ├── img/                    # SD card output (generated)
│   ├── linker.ld
│   ├── config.txt
│   ├── Makefile
│   └── README.md
├── prj_docs/
│   ├── project_notes.md
│   └── raw_requirements.md
├── std_docs/
├── Makefile                    # Root Makefile — builds all projects
└── README.md
```

---

## Build

### Build all projects

```bash
make
```

### Build a specific project

```bash
make blink       # builds blink/ only
make blinkRGB    # builds blinkRGB/ only
```

### Other targets

```bash
make dump        # disassembly listing for all projects
make clean       # remove all obj/ and img/ directories
```

### Output directories (created automatically)

| Directory | Contents |
|-----------|----------|
| `blink/obj/` | `.o` files, `kernel.elf` |
| `blink/img/` | `kernel.img`, `config.txt` — copy to SD card |
| `blinkRGB/obj/` | `.o` files, `kernel.elf` |
| `blinkRGB/img/` | `kernel.img`, `config.txt` — copy to SD card |

---

## SD Card Setup

1. Format a micro SD card as **FAT32**.

2. Download the Raspberry Pi firmware files from  
   <https://github.com/raspberrypi/firmware/tree/master/boot>  
   and copy these **three files** to the SD card root:

   | File | Purpose |
   |------|---------|
   | `bootcode.bin` | GPU first-stage bootloader |
   | `start.elf` | GPU firmware |
   | `fixup.dat` | GPU memory split configuration |

3. Copy the contents of `blink/img/` to the SD card root.
   This folder is auto-populated by `make` and already contains both `config.txt` and `kernel.img`.

The SD card root should contain:

```
/
├── bootcode.bin
├── start.elf
├── fixup.dat
├── config.txt
└── kernel.img
```

---

## How It Works

### Boot sequence

1. On power-up the GPU runs the first-stage bootloader (`bootcode.bin`).
2. The GPU loads `start.elf`, reads `config.txt`, then copies `kernel.img`  
   to physical address **`0x8000`** and releases the CPU cores.
3. All four Cortex-A53 cores start executing at `0x8000`.

### `boot.S`

- Reads `MPIDR` to obtain the CPU ID.
- Cores 1-3 are immediately parked in a `wfe` spin loop.
- Core 0 sets `sp = 0x8000` (stack grows downward below the image).
- Zeros the `.bss` section (required by the C standard).
- Calls `main()`.

### `main.c`

- Configures GPIO 29 as an output via `gpio_set_function()`.
- Toggles the pin in an infinite loop with a busy-wait delay.

### GPIO driver (`gpio.c` / `gpio.h`)

- Direct memory-mapped register access — no MMIO abstraction layer.
- `GPFSEL` registers are computed at runtime from the pin number so a  
  single function handles all 54 GPIO pins.

---

## Memory Map

| Region | Address |
|--------|---------|
| Kernel load address / stack top | `0x0000_8000` |
| Peripheral base | `0x3F00_0000` |
| GPIO base | `0x3F20_0000` |

---

## Projects

| Folder | Description | Key GPIO |
|--------|-------------|----------|
| `blink/` | On-board ACT LED blink at ~1 Hz | GPIO 29 (internal) |
| `blinkRGB/` | External RGB LED 7-colour cycle | GPIO 17, 27, 22 |

## Planned / Next Steps

- **Mini UART driver** — serial debug output over GPIO 14/15 (set `enable_uart=1` in `config.txt`)
- **ARM timer driver** — accurate delays using the BCM2837 System Timer at `0x3F003000`
- **Interrupt controller** — handle IRQs from the BCM2837 interrupt controller
- **SPI / I2C drivers** — bit-bang or register-level peripheral drivers
- **64-bit port** — switch to `aarch64-none-elf` toolchain, `kernel8.img`, and AArch64 assembly
