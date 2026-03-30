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

xtensa-esp32s3-elf-addr2line -pfiaC -e build\esp-doom.elf 0x420698D3:0x3FCA1F80 0x42069CF0:0x3FCA1FA0 0x40378AAD:0x3FCA1FD0 0x42008662:0x3FCCF800 0x42009527:0x3FCCF830 0x42008091:0x3FCCF850 0x42007BAE:0x3FCCF870 0x42070352:0x3FCCF890 0x4206F089:0x3FCCF8B0 0x4206F0E6:0x3FCCF8D0 0x4207B601:0x3FCCF900 0x4207874F:0x3FCCFA20 0x420703C9:0x3FCCFD40 0x4208A0C1:0x3FCCFD70 0x4038521D:0x3FCCFDA0 0x42061B8D:0x3FCCFDF0 0x4206909E:0x3FCCFE50 0x42068EDB:0x3FCCFE90 0x42009FDE:0x3FCCFEB0 0x4205733D:0x3FCCFEE0 0x42051710:0x3FCCFF00 0x42051A1E:0x3FCCFF30 0x420282E4:0x3FCCFF50 0x42010AE1:0x3FCCFF70 0x4200CB89:0x3FCCFFA0 0x4200CD6B:0x3FCCFFF0 0x42009C63:0x3FCD0010 0x420099D7:0x3FCD0030 0x4037E641:0x3FCD0070

---



# configurar botones:
Path:
\components\prboom-esp32-compat\gamepad.c

GPIO mapping:
PIN_UP      35
PIN_DOWN    36
PIN_LEFT    37
PIN_RIGHT   38
PIN_FIRE    39
PIN_USE     40
PIN_STRAFE  41
PIN_RUN     42












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

No se utiliza:
- ES8311
- I2C
- Touchscreen
- Hardware específico del ESP32-S3-BOX

---

## ✅ Notas importantes

- El MAX98357A no necesita MCLK.
- Funciona directamente con I2S estándar.
- Si no hay sonido, verificar:
  - Conexiones
  - Fuente de alimentación
  - Parlante
  - Masa común entre ESP32 y módulo

---

Con esta configuración el proyecto es totalmente compatible con cualquier ESP32-S3 + DAC I2S externo.
