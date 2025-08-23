# Vehicle Detection

An ESP32-based vehicle detection system using I2C magnetometers.  
Designed to detect magnetic disturbances (vehicles) and send alerts over a Meshtastic LoRa mesh.  
Building with a modular C++ structure using PlatformIO.

---

## Features
- Solar powered detection Node
- Detects vehicles passing over a buried sensor on the dirt path, at least 40cm under.
- Meshtastic-compatible alarm messaging (sends signal over I2C wire into existing Meshtastic node)

---

## Hardware SHOPPING

### Core Board
- **[ESP32-S3 Heltec WiFi LoRa 32 V3](https://www.amazon.es/-/en/Fangxuee-Heltec-Meshtastic-LoRa32-863-928MHZ/dp/B0DTHZ8CVF/)**  
- **[XIAO nRF52840 & Wio-SX1262 Kit for Meshtastic](https://core-electronics.com.au/xiao-esp32s3-meshtastic-lora-wio-sx1262-wireless-communication-device.html)**

### Magnetometers
- **Australia**
  - [MMC5603](https://littlebirdelectronics.com.au/products/adafruit-triple-axis-magnetometer-mmc5603)  
  - [MLX90393](https://littlebirdelectronics.com.au/products/adafruit-wide-range-triple-axis-magnetometer-mlx90393)  

- **Europe**
  - [MMC5603](https://es.aliexpress.com/item/1005008923222064.html?spm=a2g0o.detail.0.0.5dd4Rl9jRl9j1I&mp=1&pdp_npi=5%40dis%21EUR%21EUR%204.09%21EUR%204.09%21%21EUR%204.09%21%21%21%402103205217559088707808148e3dd2%2112000047218838031%21ct%21ES%212759432007%21%211%210&gatewayAdapt=glo2esp)
  - [MLX90393](https://es.aliexpress.com/item/1005009594077249.html?spm=a2g0o.detail.0.0.1bf3C05dC05d5Z&mp=1&pdp_npi=5%40dis%21EUR%21EUR%206.17%21EUR%203.89%21%21EUR%203.89%21%21%21%402103205217559088177796853e3dd2%2112000049565358424%21ct%21ES%212759432007%21%211%210&gatewayAdapt=glo2esp)

### I2C Extension
- [SparkFun QwiicBus Kit](https://core-electronics.com.au/sparkfun-qwiicbus-endpoint.html)
(For long-distance I2C sensor connectivity)

### Optional
- **[MAX17048G+T10](https://www.mouser.com/datasheet/2/609/MAX17048_MAX17049-3469099.pdf)**  
  Micropower lithium battery gauge with ±7.5 mV precision.

### Solar panel
- **[Soshine Mini Solar Panel](https://www.amazon.com/gp/product/B099RSLNZ4/ref=ox_sc_rp_title_rp_4?smid=&pf_rd_p=5d865192-d391-4c21-b512-33ef668eaf70&pf_rd_r=ZD34D17Q1Y9YK24T2YJF&pd_rd_wg=ZZ4WH&pd_rd_i=B099RSLNZ4&pd_rd_w=qxl9E&content-id=amzn1.sym.5d865192-d391-4c21-b512-33ef668eaf70&pd_rd_r=34bb9ed1-854c-4531-a427-4dfd883824a8&th=1)**
  USB Solar Panel Charger 5v 6w.

### Battery pack
- **[3.7V 18650 10500mAh 1S3P Li-ion Battery](https://www.amazon.com/CASFROM-Compatible-Raspberry-Expansion-Simplehuman/dp/B0F6YQ5N89/ref=sr_1_3_sspa?crid=1Y3TGZSPH0CLH&dib=eyJ2IjoiMSJ9.2cC4wHjAOveuni3fPLgklI2anq8qZR3mahU_1NZ8Wfkr_xLWDXc3jyd4yNKjtAzIJgP6rkHCaVlKyeBhPLEavPd6ATVfCvVypiUZy9Fz5wVC7qCxgWrsOkE4iVT9EJH9b3zQEnJbewSdcbDe1_evJixsyR-V9SwzzXcvsmppd3Qv9EkH1Da1TYBWlWirT4vLtMP1MdHqve-rQfHhaF4oaLit2LzsnA4UtkWgzXZwiHsscuDUr50korjJxhCqK0t619teYui8PROMIDfy_CPHgxaTLrPr0RB3jxQCIlf-bRw.tuKJZKJZRW2r4_WaC515kUx-KhOzimWtvhVpHQ-kamo&dib_tag=se&keywords=lithium+battery+pack+18650&qid=1755907422&sprefix=lithium+battery+pack+18650%2Caps%2C228&sr=8-3-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)**


---

## Existing infrastructure
- **[Solar Meshtastic Node](https://es.aliexpress.com/item/32908615100.html?spm=a2g0o.order_list.order_list_main.5.281f1802KYNsAG&gatewayAdapt=glo2esp)**


## Wiring Notes

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

## Detection Logic (DRAFT)

- Sample magnetometer once per second  
- Compare current azimuth against a smoothed average  
- If deviation > threshold (e.g. **2.5°**) → trigger event  
- On trigger:  
  ```cpp
  MeshtasticDevice.sendText("Vehicle detected at...");
  ```
