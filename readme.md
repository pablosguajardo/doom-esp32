# 🎮 ESP32-S3 DOOM Project

![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.2-blue)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-green)
![Status](https://img.shields.io/badge/Status-Working-success)

---

## 🧰 Prerequisites

* **ESP-IDF**: v5.2
* **Python**: 3.13.0
* ESP32-S3 board

---

## ⚙️ Configuration

```bash
idf.py menuconfig
```

---

## 🔄 Dependency Changes

```bash
idf.py reconfigure
```

---

## 🏗️ Build

```bash
idf.py build
```

### Clean build (recommended)

```bash
idf.py fullclean
```

---

## 🚀 Deployment (Step by Step)

### 1️⃣ Partition Table

```bash
idf.py partition-table
idf.py -p COM5 partition-table-flash
```

---

### 2️⃣ Flash WAD Files

#### CMD

```cmd
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x200000 prboom.wad
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x280000 DOOM1.WAD
```

#### PowerShell

```powershell
python "$env:IDF_PATH\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x280000 DOOM1.WAD
```

---

### 3️⃣ Flash Firmware

```bash
idf.py -p COM5 flash
```

---

## ⚡ One-Click Scripts

### Windows (.bat)

```bat
@echo off
idf.py fullclean
idf.py build
idf.py -p COM5 flash
pause
```

---

### PowerShell (.ps1)

```powershell
idf.py fullclean
idf.py build
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
0x42009e84:0x3fcd0a30
```

---

## ⚠️ Watchdog

Disable if needed:

```
Component config
  → ESP System Settings
      → [ ] Interrupt watchdog
```

---

## 🎮 Controls (GPIO)

### Directions

| Action | GPIO |
| ------ | ---- |
| UP     | 8    |
| DOWN   | 9    |
| LEFT   | 10   |
| RIGHT  | 11   |

### Main Actions

| Action | GPIO |
| ------ | ---- |
| FIRE   | 12   |
| USE    | 2    |
| RUN    | 16   |
| STRAFE | 18   |

### System

| Action | GPIO |
| ------ | ---- |
| ESC    | 13   |
| MAP    | 14   |
| ENTER  | 19   |
| PAUSE  | 20   |

---

## 🔊 Audio

| Signal | GPIO |
| ------ | ---- |
| BCLK   | 17   |
| WS     | 47   |
| DOUT   | 15   |

---

## 📺 Display

| Signal | GPIO |
| ------ | ---- |
| MOSI   | 6    |
| CLK    | 7    |
| CS     | 5    |
| DC     | 4    |
| RST    | 48   |
| BCKL   | 45   |

---

## 🧠 Notes

* Flash order matters: **partition → firmware → WAD**
* WAD must match partition offsets
* Avoid flash/PSRAM pins
* Use external power for peripherals

---

## ⭐ Tips

* Use `idf.py monitor` with `Ctrl+]` to exit
* Use `idf.py flash -v` to debug flashing
* Keep WAD flashing separate from firmware

---

🚀 Ready to run DOOM on ESP32-S3



Known Bugs
----------

- ESP32-DOOM does not support saving or loading of savegames.


Credits
-------

Doom is released by iD software in 1999 under the Gnu GPL. PrBoom is a modification of this code; its authors are credited in 
the ``components/prboom/AUTHORS`` file. The ESP32 modifications are done by Espressif and licensed under the Apache license, version 2.0.


License
-------

This project is open source and freely available for anyone to use, modify, and distribute.

You are welcome to:
- Use the code for personal or commercial projects
- Modify and adapt it to your needs
- Share improvements with the community

This repository is provided “as is”, without warranty of any kind.

If you find this project useful, contributions, feedback, and improvements are always welcome.