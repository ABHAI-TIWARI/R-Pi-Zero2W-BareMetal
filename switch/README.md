# switch — Tactile Switch RGB LED Controller

Bare-metal Raspberry Pi Zero 2W project.  
Two tactile switches cycle through 7 colours on a common-cathode RGB LED — no OS, no libraries, just direct register access.

---

## Hardware

### RGB LED

| Channel | GPIO | Physical Pin | Resistor |
|---------|------|-------------|----------|
| Red     | 17   | 11          | 330 Ω    |
| Green   | 27   | 13          | 330 Ω    |
| Blue    | 22   | 15          | 330 Ω    |

- Common cathode → connect cathode to GND.
- Each anode through a 330 Ω resistor to its GPIO pin.
- GPIO HIGH = LED channel ON.

### Tactile Switches

| Switch | GPIO | Physical Pin | Pull-up | Action         |
|--------|------|-------------|---------|----------------|
| SW1    | 16   | 36          | 4.7 kΩ  | Next colour →  |
| SW2    | 20   | 38          | 4.7 kΩ  | Previous colour ← |

- External 4.7 kΩ pull-up to 3.3 V on each switch pin.
- Other terminal of switch → GND.
- Active LOW: pin reads 0 when pressed, 1 when released.

### Wiring Diagram

```
3.3V ──┬─── 4.7kΩ ─── GPIO16 (SW1) ─── [Switch 1] ─── GND
       └─── 4.7kΩ ─── GPIO20 (SW2) ─── [Switch 2] ─── GND

GPIO17 ─── 330Ω ─── Red   anode  ┐
GPIO27 ─── 330Ω ─── Green anode  ├── RGB LED (common cathode) ─── GND
GPIO22 ─── 330Ω ─── Blue  anode  ┘
```

---

## Colour Cycle

| Index | Colour  | Red | Green | Blue |
|-------|---------|:---:|:-----:|:----:|
| 0     | Red     |  ●  |       |      |
| 1     | Green   |     |   ●   |      |
| 2     | Blue    |     |       |  ●   |
| 3     | Yellow  |  ●  |   ●   |      |
| 4     | Cyan    |     |   ●   |  ●   |
| 5     | Magenta |  ●  |       |  ●   |
| 6     | White   |  ●  |   ●   |  ●   |

Starts at **Red** on power-up. SW1 steps forward, SW2 steps backward — both wrap around.

---

## How It Works

### Boot sequence (`boot.S`)
1. CPU starts at `0x8000` (all 4 cores).
2. Cores 1–3 are parked in a WFE loop — only core 0 continues.
3. Stack pointer set to `0x8000` (grows downward, below the image).
4. BSS section zeroed.
5. `main()` called.

### Main loop (`main.c`)
1. Configure GPIO 17, 27, 22 as **outputs** (RGB LED).
2. Configure GPIO 16, 20 as **inputs** (switches, external pull-ups).
3. Set LED to Red (index 0).
4. Poll SW1 and SW2 continuously:
   - On LOW detected → wait ~2.5 ms debounce → re-check → change colour → wait for release → debounce release.

### Debounce
- `DEBOUNCE_TICKS = 375,000` NOP cycles ≈ **2.5 ms** at 600 MHz.
- Applied on both press and release edges to suppress contact bounce.

---

## Project Structure

```
switch/
├── src/
│   ├── boot.S      — AArch32 startup, core parking, BSS zero, call main
│   ├── gpio.h      — Register map, pin defines, driver API
│   ├── gpio.c      — gpio_set_function, gpio_set, gpio_clear, gpio_read
│   └── main.c      — Colour table, switch polling, debounce logic
├── linker.ld       — Places image at 0x8000, exports __bss_start/__bss_end
├── Makefile        — Builds to obj/ and img/
├── config.txt      — RPi boot config (arm_64bit=0, kernel=kernel.img)
└── README.md       — This file
```

Build outputs:

```
switch/
├── obj/
│   ├── boot.o
│   ├── gpio.o
│   └── main.o
└── img/
    ├── kernel.elf  — ELF with debug info
    ├── kernel.img  — Raw binary for SD card
    └── config.txt  — Copy of boot config
```

---

## Build

From the repo root:
```bash
make switch
```

Or from inside the `switch/` folder:
```bash
make
```

Clean:
```bash
make clean
```

Disassembly dump:
```bash
make dump        # writes obj/kernel.dump
```

**Toolchain required:** `arm-none-eabi-gcc` (AArch32, tested with 10.3.1)

---

## Flash to SD Card

1. Format SD card as **FAT32** (use `guiformat.exe` for cards > 32 GB).
2. Copy firmware files to SD card root:
   - `bootcode.bin`
   - `start.elf`
   - `fixup.dat`
   
   Download from: https://github.com/raspberrypi/firmware/tree/master/boot

3. Copy build outputs:
   ```
   switch/img/kernel.img  →  SD card root
   switch/img/config.txt  →  SD card root
   ```

4. Eject, insert into Pi Zero 2W, power on.

---

## Memory Map

| Address    | Contents                     |
|------------|------------------------------|
| `0x0000`   | Exception vectors (unused)   |
| `0x8000`   | `_start` — kernel entry point |
| `0x8000+`  | `.text`, `.rodata`, `.data`, `.bss` |
| `0x3F200000` | GPIO peripheral registers  |
