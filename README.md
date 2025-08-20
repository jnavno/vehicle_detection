# Vehicle Detection

An ESP32-based vehicle detection system using I2C magnetometers.  
Designed to detect magnetic disturbances (vehicles) and send alerts over a Meshtastic LoRa mesh.  
Building with a modular C++ structure using PlatformIO.

---

## ✨ Features
- Solar powered detection Node
- Detects vehicles passing over a buried sensor on the dirt path, at least 40cm under.
- Meshtastic-compatible alarm messaging

---

## 🛠 Hardware SHOPPING

### Core Board
- **[ESP32-S3 Heltec WiFi LoRa 32 V3](https://www.amazon.es/-/en/Fangxuee-Heltec-Meshtastic-LoRa32-863-928MHZ/dp/B0DTHZ8CVF/)**  

### Magnetometers
- **Australia**
  - [MMC5603](https://littlebirdelectronics.com.au/products/adafruit-triple-axis-magnetometer-mmc5603)  
  - [MLX90393](https://littlebirdelectronics.com.au/products/adafruit-wide-range-triple-axis-magnetometer-mlx90393)  

- **Europe**
  - [MMC5603](https://www.amazon.es/-/en/dp/B0F99P53SB)  
  - [MLX90393](https://www.amazon.es/-/en/dp/B0F99P53SB)  

### I2C Extension
- [SparkFun QwiicBus Kit](https://littlebirdelectronics.com.au/products/sparkfun-qwiicbus-kit)  
(For long-distance I2C sensor connectivity)

### Optional
- **[MAX17048G+T10](https://www.mouser.com/datasheet/2/609/MAX17048_MAX17049-3469099.pdf)**  
  Micropower lithium battery gauge with ±7.5 mV precision.

### Solar panel
- **[Soshine Mini Solar Panel](https://www.amazon.com/gp/product/B099RSLNZ4/ref=ox_sc_rp_title_rp_4?smid=&pf_rd_p=5d865192-d391-4c21-b512-33ef668eaf70&pf_rd_r=ZD34D17Q1Y9YK24T2YJF&pd_rd_wg=ZZ4WH&pd_rd_i=B099RSLNZ4&pd_rd_w=qxl9E&content-id=amzn1.sym.5d865192-d391-4c21-b512-33ef668eaf70&pd_rd_r=34bb9ed1-854c-4531-a427-4dfd883824a8&th=1)**
  USB Solar Panel Charger 5v 6w.

### Battery pack
- **[3.7V 4400mAh ICR18650](https://www.amazon.com/Coonyard-ICR18650-Rechargeable-Electronics-Equipment/dp/B0BJKFJ227/ref=sr_1_2_sspa?crid=1Y3TGZSPH0CLH&dib=eyJ2IjoiMSJ9.n97eN3WSFAfckpNQOrRRglA0sQHbViHLMtRlAhp8RxC11dZAUCuWR3ziV-ZXb-54bbs8kOv-UI626IEuJQupILSqPdCm95ZorcwXnY_NnJai6oZnf6lWl-MHITtGJwjBcy_E_k7vgJQWUJiZiW87cbXNxgVX-CjlX_d8Jyn4qYHhpud7R2UZHiDAOvbCJ7ucZzxqx-Dd4SzTirNugyzjOlAzNNVUUBnLsdQXRNk6oAdjyi0wZGtcD_Qd7fQkSwEvmphmDnEWJWpsVP6Fi1y6rIAluU8jn4C5pyXbs5YH9mA.Jmv02-Eyv-_iGgH7XSgxcg3Jj9E1dHRRPSx0lIPCaYg&dib_tag=se&keywords=lithium+battery+pack+18650&qid=1755726874&sprefix=lithium+battery+pack+18650%2Caps%2C228&sr=8-2-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)**

---

## 🔌 Wiring Notes

- **ESP32-S3 Heltec WiFi LoRa 32 V3**  
  - Has **Vext (3.3V)** output controlled by GPIO36, which is great for powering the external sensors and save energy during deep sleep.  

- **MMC5603**  
  - Has **high resolution (0.0625 mG/LSB)** but requires careful initialization since readings can fluctuate without proper set/reset cycles.  

- **MLX90393**  
  - Very flexible with multiple operating modes and ranges, but can draw more current than the MMC5603 if left continuously active, must put to sleep before esp goes to sleep.  

- **SparkFun QwiicBus Kit**  
  - Allows **long-distance I2C** over twisted pair cable (ethernet cable), but both ends need a transceiver board. Voltage-level must match (either 3.3V or 5V).  

- **MAX17048G+T10 (optional)**  
  - Easy to use (no sense resistor needed). Accuracy improves after **several charge/discharge cycles** for learning the battery profile. Much better precision than on board heltec. 

---

## 📡 Detection Logic (DRAFT)

- Sample magnetometer once per second  
- Compare current azimuth against a smoothed average  
- If deviation > threshold (e.g. **2.5°**) → trigger event  
- On trigger:  
  ```cpp
  MeshtasticDevice.sendText("Vehicle detected at...");

