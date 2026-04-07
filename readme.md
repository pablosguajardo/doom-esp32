---
# para compilar prerequisitos:
ESP-IDF v5.2

Python 3.13.0





---
# menu de configuracion:
idf.py menuconfig


---
# al sacar y/o agregar dependencias::
idf.py reconfigure


---
# compilar
idf.py build

---
# para recompilar 1ro:
idf.py fullclean




## Importante seguir estos pasos para el despliegue:
---
# particiones:
idf.py partition-table
despues:
idf.py -p COM5 partition-table-flash

---
# cmd para  copiar wad:
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x200000 prboom.wad
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x280000 DOOM1.WAD

powershell:
python "$env:IDF_PATH\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x280000 DOOM1.WAD

---
# deployar:
idf.py -p COM5 flash

---
# Para debug:
idf.py -p COM5 monitor
---
# Para ver errores:

xtensa-esp32s3-elf-addr2line -pfiaC -e build\esp-doom.elf 0x40056f72:0x3fcd0a20 0x42009e84:0x3fcd0a30 0x4205646c:0x3fcd0a50 0x42056744:0x3fcd0a90 0x4200ba57:0x3fcd0ab0 0x4200bd17:0x3fcd0b00 0x42009a47:0x3fcd0b20 0x42009677:0x3fcd0b40 0x4037d87d:0x3fcd0b80

---
# Watchdog config
Component config
  → ESP System Settings
      → [ ] Interrupt watchdog   (desactivarlo)


---
# configurar botones:
Path:
\components\prboom-esp32-compat\gamepad.c

GPIO mapping:
// Directions
PIN_UP              8
PIN_DOWN            9
PIN_LEFT            10
PIN_RIGHT           11
// Main actions
PIN_FIRE            12
PIN_USE             2
PIN_RUN             16
PIN_STRAFE          18
// System
PIN_ESCAPE          13
PIN_MAP             14
PIN_MENU_ENTER      19
PIN_PAUSE           20
// Remaining buttons (using truly free pins)
PIN_WEAPONTOGGLE    1
PIN_STRAFELEFT      3
PIN_STRAFERIGHT     46


# audio:
GPIO mapping:
BCLK  = 17
WS    = 47
DOUT  = 15
MCLK  = 2 (no utilizado por MAX98357A)


# pantalla:
GPIO mapping:
PIN_NUM_MISO -1
PIN_NUM_MOSI 6
PIN_NUM_CLK  7
PIN_NUM_CS   5
PIN_NUM_DC   4
PIN_NUM_RST  48
PIN_NUM_BCKL 45




