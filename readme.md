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
# para 1ro recompilar:
idf.py fullclean




## IMportante seguir estos pasos para el dspliegue:
---
# particiones:
idf.py partition-table
dspues:
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

xtensa-esp32s3-elf-addr2line -pfiaC -e build\esp-doom.elf 0x4037A1E6:0x3FCA1670 0x40378995:0x3FCA1690 0x42027571:0x3FCCD5C0 0x42027511:0x3FCCD5E0 0x4202940C:0x3FCCD600 0x40377DC7:0x3FCCD620 0x42054496:0x3FCCD700 0x4200A421:0x3FCCD720 0x4200AE6D:0x3FCCD740 0x4200B1CC:0x3FCCD760 0x4200BDEE:0x3FCCD780 0x42009B13:0x3FCCD7A0 0x420098A7:0x3FCCD7C0 0x4037D8D1:0x3FCCD800

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
PIN_UP      8
PIN_DOWN    9
PIN_LEFT    10
PIN_RIGHT   11
PIN_FIRE    12
PIN_USE     13
PIN_STRAFE  14
PIN_RUN     16


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





---

# 🔊 Configuración de Audio con MAX98357A (ESP32-S3-N16R8)

Este proyecto fue adaptado para funcionar con un módulo I2S externo **MAX98357A**  

El driver de audio utiliza **I2S estándar**, sin codec ES8311 ni I2C.

---

## ✅ Hardware necesario

- ESP32-S3 (ej: S3-N16R8)
- Módulo **MAX98357A I2S DAC + Amplificador**
- Parlante 4Ω u 8Ω

---

## ✅ Conexión recomendada

Configuración actual en `sndhw.c`:

```
BCLK  = GPIO17
WS    = GPIO47
DOUT  = GPIO15
MCLK  = GPIO2 (no utilizado por MAX98357A)
```

Conectar así:

| ESP32-S3 | MAX98357A |
|-----------|-----------|
| GPIO17    | BCLK      |
| GPIO47    | LRC (WS)  |
| GPIO15    | DIN       |
| 5V o 3.3V | VIN       |
| GND       | GND       |

Opcional:
- SD → GND (siempre activo)
- O SD → GPIO para control de mute

---

## ✅ Configuración en el Proyecto

No se requiere configuración adicional en `menuconfig`.

El driver I2S ya está implementado en:

```
components/prboom-esp32-compat/sndhw.c
```

El flujo de audio es:

```
Doom → snd_cb → audio_task → i2s_channel_write → MAX98357A → Parlante
```
