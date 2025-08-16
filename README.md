# Vehicle Detection

ESP32-based vehicle detection system using the QMC5883L magnetometer. Designed to detect magnetic disturbances (vehicles) and send alerts over Meshtastic LoRa mesh. Modular C++ structure using PlatformIO.

## Features

Europe:
**[ESP32-S3 Heltec WiFi LoRa 32 V3](https://www.amazon.es/-/en/Fangxuee-Heltec-Meshtastic-LoRa32-863-928MHZ/dp/B0DTHZ8CVF/ref=sr_1_2?crid=MVLM5671HX8I&dib=eyJ2IjoiMSJ9.nje33bFH0ZrnNsVPglKwJJ5LXJJCCCkm9hLy5Ymdsal0aqW7maEbIXSYaxIISYZkmGVrwXnR9H4bUjLGRYhEPoPEY0NSaJqF3BC4z52oBvG8uE409-BP-4BDdWzB0CekjDiDY6CW8Czj3t-0F1JxJAZq0Jt-ZiCmTSzqpBOt2pRYS8W14WvNeiF3vv8plI3QYhkJUmgpQGkwrwqaObwd1Rwb_jDEgCUlXYCT4fr3BI4ES2eDo7s-_lKhZUo_GGKaOV0dNj_TCpzGDOG60hqJyVffCC-Ji9pnnfMd5ZFutjw.8VoTQWg_or2dymqRCSLk4o4hpTMBjKRoT7bCZJNT0-I&dib_tag=se&keywords=ESP32-S3+Heltec+WiFi+LoRa+32+V3&qid=1755363013&sprefix=esp32-s3+heltec+wifi+lora+32+v3%2Caps%2C285&sr=8-2)**



Australia:
Option1:
**[MMC5603](https://littlebirdelectronics.com.au/products/adafruit-triple-axis-magnetometer-mmc5603?_pos=1&_sid=301f87019&_ss=r)**
Option 2:
**[MLX90393](https://littlebirdelectronics.com.au/products/adafruit-wide-range-triple-axis-magnetometer-mlx90393?_pos=1&_sid=c0fe86ecc&_ss=r)**

Europe:
**[MMC5603](https://www.amazon.es/-/en/dp/B0F99P53SB/ref=sr_1_1?crid=QQG4R9B2NWEB&dib=eyJ2IjoiMSJ9.Vnw-VOezcMEqnNAVGDgnHQ.BytMddSIGMFqXp1WcGHyoP5Mwe-VVLRjPvo9zcxvJec&dib_tag=se&keywords=MMC5603&qid=1755362765&sprefix=mmc5603%2Caps%2C261&sr=8-1)**

**[MLX90393](https://www.amazon.es/-/en/dp/B0F99P53SB/ref=sr_1_1?crid=1UC062GLW7I4M&dib=eyJ2IjoiMSJ9.Vnw-VOezcMEqnNAVGDgnHQ.BytMddSIGMFqXp1WcGHyoP5Mwe-VVLRjPvo9zcxvJec&dib_tag=se&keywords=MMC5603&qid=1755362700&sprefix=%2Caps%2C880&sr=8-1)**

I2C transducer (for long distance, I2C-based sensor connectivity)
https://littlebirdelectronics.com.au/products/sparkfun-qwiicbus-kit?_pos=3&_sid=89eb868b5&_ss=r

- **[MAX17048G+T10](https://www.mouser.com/datasheet/2/609/MAX17048_MAX17049-3469099.pdf)**: Micropower current lithium battery gauge with ±7.5mV precision.
- Modular C++ codebase
- Meshtastic-compatible alarm messaging

## Hardware

| Component                        | ESP32 Pin   |
|----------------------------------|-------------|
| QMC5883L SDA                     | GPIO 41     |
| QMC5883L SCL                     | GPIO 42     |
| QMC5883L VCC                     | Vext (3.3V) |
| QMC5883L GND                     | GND         |
| Optional: MAX17048 SDA/SCL      | 41 / 42     |

## Wiring Notes

- QMC5883L uses I2C address `0x0D`
- Vext (GPIO36-controlled) powers the sensor
- SDA/SCL must be defined as GPIO 41/42 on Heltec V3


## Detection Logic

- Reads magnetometer every second
- Compares current azimuth vs smoothed average
- If deviation > threshold (e.g. 2.5°), triggers event
- Sends `MeshtasticDevice.sendText("Vehicle detected at...")`
- Uses cooldown to avoid spam
