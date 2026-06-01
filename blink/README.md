# blink — ACT LED Blink

Blinks the on-board green ACT LED (GPIO 29) at ~1 Hz.  
No OS, no HAL, no standard library.

---

## Hardware

| Item | Detail |
|------|--------|
| Board | Raspberry Pi Zero 2W |
| SoC | BCM2710A1 (quad-core Cortex-A53) |
| LED | On-board green ACT LED |
| GPIO | 29 (internal — not on the 40-pin header) |
| Peripheral base | `0x3F000000` |

No external components needed.

---

## Project Structure

```
blink/
├── src/
│   ├── boot.S      — AArch32 startup: park cores 1-3, zero BSS, call main()
│   ├── main.c      — LED blink loop
│   ├── gpio.c      — GPIO driver (GPFSEL, GPSET, GPCLR, GPLEV)
│   └── gpio.h      — BCM2710A1 register map + API
├── obj/            — Build artefacts (generated): .o files, kernel.elf
├── img/            — SD card output (generated): kernel.img, config.txt
├── linker.ld       — Kernel loaded at 0x8000
├── Makefile
└── README.md
```

---

## Build

From the repo root:

```powershell
make
```

Or from inside this folder:

```powershell
cd blink
make
```

Output: `img/kernel.img` (flat binary, ~428 bytes)

```
make dump    # disassembly → obj/kernel.dump
make clean   # remove obj/ and img/
```

---

## Flash to SD Card

Copy the entire `img/` folder contents to the FAT32 SD card root:

```powershell
Copy-Item "img\kernel.img", "img\config.txt" F:\
```

The SD card root must also contain (download from the [RPi firmware repo](https://github.com/raspberrypi/firmware/tree/master/boot)):

```
/
├── bootcode.bin
├── start.elf
├── fixup.dat
├── config.txt      ← from img/
└── kernel.img      ← from img/
```

> **SD card must be formatted as FAT32.** Use [guiformat](http://ridgecrop.co.uk/guiformat.exe) on Windows for cards larger than 32 GB.

---

## How It Works

```
boot.S   → parks cores 1–3, zeroes BSS, calls main()
main()   → gpio_set_function(29, OUTPUT)
         → loop: gpio_set(29) → delay ~1s → gpio_clear(29) → delay ~1s
```

The delay is a busy-wait NOP loop (`150 000 000` ticks ≈ 1 s at 600 MHz).  
For accurate timing, replace with the BCM2837 System Timer at `0x3F003000`.
