# RPi Zero 2W Bare-Metal — Project Notes
> Generated: 2026-05-27

---

## Project Summary

Bare-metal C + ARM Assembly for Raspberry Pi Zero 2W.  
Blinks ACT LED (GPIO 29) at ~1 Hz. No OS, no HAL, no stdlib.

---

## Project Structure

```
RaspberryPi-Zero-Projects/
├── src/
│   ├── boot.S       — AArch32 startup: park cores 1-3, zero BSS, call main()
│   ├── main.c       — LED blink loop
│   ├── gpio.c       — GPIO driver (GPFSEL, GPSET, GPCLR, GPLEV)
│   └── gpio.h       — BCM2710A1 register map + API
├── linker.ld        — kernel loaded at 0x8000
├── Makefile         — arm-none-eabi-gcc → kernel.elf → kernel.img
├── config.txt       — arm_64bit=0, kernel=kernel.img
├── prj_docs/
│   ├── raw_requirements.md
│   └── project_notes.md   ← this file
└── README.md
```

---

## Hardware

| Item | Detail |
|------|--------|
| Board | Raspberry Pi Zero 2W |
| SoC | BCM2710A1 (quad-core ARM Cortex-A53) |
| Execution mode | AArch32 (32-bit ARM) |
| ACT LED pin | GPIO 29 |
| Peripheral base | `0x3F000000` |
| GPIO base | `0x3F200000` |
| Kernel load address | `0x8000` |

---

## Toolchain (Windows)

| Tool | Version | Location |
|------|---------|----------|
| `arm-none-eabi-gcc` | 10.3.1 (2021.10) | `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin` |
| `make` | GNU Make 4.4.1 | `C:\NXP\S32DS.3.5\S32DS\build_tools\msys32\usr\bin\make.exe` |

Both are already present in the User PATH registry entry.

---

## Build Commands

```powershell
# Refresh PATH in the current PowerShell session (needed in new terminals)
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("PATH","User")

# Build
make

# OR call make by full path if it still isn't found:
& "C:\NXP\S32DS.3.5\S32DS\build_tools\msys32\usr\bin\make.exe"

# Disassembly listing (debugging)
make dump

# Clean build artefacts
make clean
```

Build output: `kernel.img` — 428 bytes flat binary.

---

## SD Card Setup

1. Format a micro SD card as **FAT32**.
2. Download from <https://github.com/raspberrypi/firmware/tree/master/boot> and copy to SD root:
   - `bootcode.bin`
   - `start.elf`
   - `fixup.dat`
3. Copy `config.txt` and `kernel.img` from this repo to SD root.

SD card root layout:
```
/
├── bootcode.bin
├── start.elf
├── fixup.dat
├── config.txt
└── kernel.img
```

---

## Boot Time

| Stage | Duration |
|-------|----------|
| Power-on + PMIC settle | ~50 ms |
| GPU bootcode.bin | ~200–400 ms |
| GPU start.elf + config.txt parse | ~200–400 ms |
| kernel.img copy (428 bytes) | < 1 ms |
| boot.S + main() first instruction | < 1 µs |
| **Total to first LED blink** | **~500 ms – 1 s** |

Speed-up tip — add to `config.txt`:
```ini
boot_delay=0
disable_splash=1
```

---

## Windows PATH Issue

The User PATH variable exceeded Windows' 2047-character GUI limit.  
**Cannot edit via System Properties dialog.**

### Fix — add entries via Python (bypasses the limit):
```python
import winreg
key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, 'Environment', 0,
                     winreg.KEY_READ | winreg.KEY_WRITE)
cur, _ = winreg.QueryValueEx(key, 'PATH')
winreg.SetValueEx(key, 'PATH', 0, winreg.REG_EXPAND_SZ, cur + r';C:\new\path')
winreg.CloseKey(key)
```

### Fix — refresh PATH in current PowerShell session:
```powershell
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
            [System.Environment]::GetEnvironmentVariable("PATH","User")
```

---

## Next Steps

- [ ] Mini UART driver — serial debug output (GPIO 14/15, set `enable_uart=1`)
- [ ] ARM System Timer driver — accurate delays (`0x3F003000`)
- [ ] Interrupt controller — handle IRQs from BCM2837 interrupt controller
- [ ] SPI / I2C drivers
- [ ] Port to AArch64 (`aarch64-none-elf`, `kernel8.img`)
