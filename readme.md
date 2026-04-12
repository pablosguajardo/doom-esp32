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

## ESP-IDF

For ESP-IDF You can find the getting started guide [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).  

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

### Set target (It is no longer necessary):

```bat
idf.py set-target esp32s3
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
xtensa-esp32s3-elf-addr2line -pfiaC -e build\esp-doom.elf 0x4037DBA6:0x3FCA72F0 0x4037BCBD:0x3FCA7310 0x420724EA:0x3FCADA40 0x4206C7FD:0x3FCADA60 0x4037B3F9:0x3FCADA80 0x4207A3B6:0x3FCADAF0 0x4207954E:0x3FCADB40 0x4207986B:0x3FCADB60 0x42079995:0x3FCADB80 0x42079A0F:0x3FCADC70 0x420799DE:0x3FCADC90 0x420799DE:0x3FCADCB0 0x420799DE:0x3FCADCD0 0x420799DE:0x3FCADCF0 0x420799DE:0x3FCADD10 0x420799DE:0x3FCADD30 0x420799DE:0x3FCADD50 0x420799DE:0x3FCADD70 0x420799DE:0x3FCADD90 0x420799DE:0x3FCADDB0 0x420799DE:0x3FCADDD0 0x420799DE:0x3FCADDF0 0x4206BDDE:0x3FCADE10 0x42026F3E:0x3FCADE30 0x420271B0:0x3FCADE50 0x42027DDE:0x3FCADE70 0x4200B01F:0x3FCADE90 0x4200A813:0x3FCADEB0 0x40382B99:0x3FCADEF0
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