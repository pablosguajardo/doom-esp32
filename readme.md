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

## Run Esp-Idf manually
```bat
C:\esp\v5.2\esp-idf\export.bat
```

## ⚙️ App Configuration

```bat
idf.py menuconfig
```

---

## 🔄 Dependency Changes

```bat
idf.py reconfigure
```

---

## 🏗️ Build

```bat
idf.py build
```

### Clean build (recommended)

```bat
idf.py fullclean
```

### Or run both:

```bat
idf.py fullclean build
```

---

## 🚀 Deployment (Step by Step)

### 1️- Partition Table

```bat
idf.py partition-table
idf.py -p COM5 partition-table-flash
```

---

### 2️- Flash WAD Files

#### CMD

```cmd
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x200000 prboom.wad
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x280000 DOOM1.WAD
```

#### PowerShell

```powershell
python "$env:IDF_PATH\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x200000 prboom.wad
python "$env:IDF_PATH\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 write_flash 0x280000 DOOM1.WAD
```

---

### 3️- Flash Firmware

```bat
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

```bat
idf.py -p COM5 monitor
```

---

### Decode Backtrace

```bat
xtensa-esp32s3-elf-addr2line -pfiaC -e build\esp-doom.elf 0x40375e16:0x3fcb1690 0x40382039:0x3fcb16b0 0x4038978d:0x3fcb16d0 0x40376e63:0x3fcb1740 0x4037704a:0x3fcb1760 0x420a7e61:0x3fcb1780 0x420a4fdb:0x3fcb18a0 0x4209cc55:0x3fcb1bc0 0x420b7aad:0x3fcb1bf0 0x4038963d:0x3fcb1c20 0x4207bea5:0x3fcb1c70 0x420812e2:0x3fcb1cd0 0x4208108f:0x3fcb1d10 0x4200b207:0x3fcb1d30 0x42071589:0x3fcb1d60 0x4206b904:0x3fcb1d80 0x4206bc12:0x3fcb1db0 0x42044548:0x3fcb1dd0 0x4202b5e1:0x3fcb1df0 0x42027599:0x3fcb1e20 0x4202777b:0x3fcb1e70 0x4200ae67:0x3fcb1e90 0x4200a6ad:0x3fcb1eb0 0x40382b99:0x3fcb1ef0
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
## 🎮 Controls

SET: HW_JOYSTICK_OPTION 
Default: 2
Available options: 
if you set it to 1, it compiles to be used with GPIO buttons; 
if you set it to 2, it compiles to use a joystick via Bluetooth.

run:
idf.py menuconfig

 ESP32-Doom platform-specific configuration →
	(2) Options: 1=GPIO buttons, 2= BT joystick  

### Bluetooth Configuration (HW_JOYSTICK_OPTION = 2)

---
🔧 Required configuration (VERY important)

Run:

idf.py menuconfig


📍 Component config
  -> Bluetooth
     [*] Bluetooth
     Host ---> NimBLE - BLE only
     Controller ---> Enabled


---

### (GPIO)Configuration (HW_JOYSTICK_OPTION = 1)

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

## Known Bugs
----------

- ESP32-DOOM does not support saving or loading of savegames.


## Credits
-------

Doom is released by iD software in 1999 under the Gnu GPL. PrBoom is a modification of this code; its authors are credited in 
the ``components/prboom/AUTHORS`` file. The ESP32 modifications are done by Espressif and licensed under the Apache license, version 2.0.


## License
-------

This project is open source and freely available for anyone to use, modify, and distribute.

You are welcome to:
- Use the code for personal or commercial projects
- Modify and adapt it to your needs
- Share improvements with the community

This repository is provided “as is”, without warranty of any kind.

If you find this project useful, contributions, feedback, and improvements are always welcome.

---

🚀 Ready to run DOOM on ESP32-S3