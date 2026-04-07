# ESP32-S3 DOOM Project Setup & Deployment Guide

---

## 🧰 Prerequisites

* **ESP-IDF**: v5.2
* **Python**: 3.13.0

---

## ⚙️ Configuration

Open project configuration menu:

```bash
idf.py menuconfig
```

---

## 🔄 Dependency Changes

After adding or removing components:

```bash
idf.py reconfigure
```

---

## 🏗️ Build

Compile the project:

```bash
idf.py build
```

### Clean build (recommended before major changes)

```bash
idf.py fullclean
```

---

## 🚀 Deployment Process (IMPORTANT)

### 1. Generate & Flash Partition Table

```bash
idf.py partition-table
idf.py -p COM5 partition-table-flash
```

---

### 2. Flash WAD Files

#### CMD:

```cmd
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x200000 prboom.wad

python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x280000 DOOM1.WAD
```

#### PowerShell:

```powershell
python "$env:IDF_PATH\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x280000 DOOM1.WAD
```

---

### 3. Flash Firmware

```bash
idf.py -p COM5 flash
```

---

## 🐞 Debugging

### Serial Monitor

```bash
idf.py -p COM5 monitor
```

---

### Decode Backtrace

```bash
xtensa-esp32s3-elf-addr2line -pfiaC -e build\esp-doom.elf \
0x40056f72:0x3fcd0a20 \
0x42009e84:0x3fcd0a30 \
0x4205646c:0x3fcd0a50 \
0x42056744:0x3fcd0a90 \
0x4200ba57:0x3fcd0ab0 \
0x4200bd17:0x3fcd0b00 \
0x42009a47:0x3fcd0b20 \
0x42009677:0x3fcd0b40 \
0x4037d87d:0x3fcd0b80
```

---

## ⚠️ Watchdog Configuration

Disable interrupt watchdog:

```
Component config
  → ESP System Settings
      → [ ] Interrupt watchdog
```

---

## 🎮 Input Configuration

**File:**

```
components/prboom-esp32-compat/gamepad.c
```

### GPIO Mapping

#### Directions

```
UP      → GPIO 8
DOWN    → GPIO 9
LEFT    → GPIO 10
RIGHT   → GPIO 11
```

#### Main Actions

```
FIRE    → GPIO 12
USE     → GPIO 2
RUN     → GPIO 16
STRAFE  → GPIO 18
```

#### System

```
ESCAPE      → GPIO 13
MAP         → GPIO 14
MENU_ENTER  → GPIO 19
PAUSE       → GPIO 20
```

#### Additional Buttons

```
WEAPONTOGGLE → GPIO 1
STRAFELEFT   → GPIO 3
STRAFERIGHT  → GPIO 46
```

---

## 🔊 Audio Configuration

```
BCLK → GPIO 17
WS   → GPIO 47
DOUT → GPIO 15
MCLK → GPIO 2 (not used by MAX98357A)
```

---

## 📺 Display Configuration

```
MISO  → -1
MOSI  → GPIO 6
CLK   → GPIO 7
CS    → GPIO 5
DC    → GPIO 4
RST   → GPIO 48
BCKL  → GPIO 45
```

---

## 🧠 Notes

* Always flash **partition table before WAD files**
* WAD offsets must match partition table configuration
* Avoid using flash/PSRAM GPIOs
* Use external power for peripherals (display/audio)

---
