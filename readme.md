



para compilar:
ESP-IDF v5.2

Python 3.13.0





menu de configuracion:
idf.py menuconfig


compilar
idf.py build

para 1ro recompilar:
idf.py fullclean




## IMportante seguir estos pasos para el dspliegue:
particiones:
idf.py partition-table
dspues:
idf.py -p COM5 partition-table-flash


cmd para  copiar wad:
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x200000 prboom.wad
python "%IDF_PATH%\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x280000 DOOM1.WAD

powershell:
python "$env:IDF_PATH\components\esptool_py\esptool\esptool.py" --chip esp32s3 --port COM5 --baud 230400 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x280000 DOOM1.WAD


deployar:
idf.py -p COM5 flash