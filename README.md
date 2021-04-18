# libi2c
I2C library and tools for ESP32 

## Setting up
Clone this repo into _lib_ folder inside PIO workspace
```
cd lib
git clone https://github.com/mc-cat-tty/libi2c
cd ..
```

## BMP280
How to extract compensation fields
1. download BME/BMP280 datasheet
2. `pdftotext BST-BMP280-DS001-11.pdf -f 21 -l 21`
3. leave only comprensation parameters table on that text file
4. `cat BST-BMP280-DS001-11.txt | grep dig_ -A2 | tr "\n" " " | tr "\-\-" "\n" | sed -e "s/unsigned short/uint16_t/" | sed -e "s/signed short/int16_t/"`