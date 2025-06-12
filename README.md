# Vehicle Detection

ESP32-based vehicle detection system using the QMC5883L magnetometer. Designed to detect magnetic disturbances (vehicles) and send alerts over Meshtastic LoRa mesh. Modular C++ structure using PlatformIO.

## Features

- ESP32-S3 Heltec WiFi LoRa 32 V3
**[QMC5883L0](https://www.amazon.com/-/es/QMC5883L-magn%C3%A9tico-electr%C3%B3nica-compatible-HMC5883L/dp/B008V9S64E/ref=sims_dp_d_dex-popular-subs-t2-v7_d_sccl_2_1/145-5063194-9708904?pd_rd_w=SwBQT&content-id=amzn1.sym.77af0fca-e4b3-4b62-98e0-b6b037709806&pf_rd_p=77af0fca-e4b3-4b62-98e0-b6b037709806&pf_rd_r=RC6CHMCQRYYAXN9TZNCX&pd_rd_wg=7EkFu&pd_rd_r=1971c322-d845-4361-8f9d-ab658386a9ae&pd_rd_i=B008V9S64E&psc=1)**-based heading disturbance detection

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
