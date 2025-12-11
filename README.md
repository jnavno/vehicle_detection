# Vehicle Detection

An ESP32-based vehicle detection system using an I2C magnetometer.
Designed to detect magnetic disturbances (vehicles) and send alerts over a Meshtastic LoRa mesh.  
Building with a modular C++ structure using PlatformIO.

---

## Features
- Solar powered detection Node
- Detects vehicles passing over a buried sensor on the dirt path, at least 40cm under.
- Meshtastic-compatible alarm messaging. Sends alarm text message signal over RS485 20 - 10m wire into existing Meshtastic node and into the Mesh.

---

## Hardware shopping (UPDATED)

### Solar panel, option 1:
- **[5V5 6W](https://core-electronics.com.au/monocrystalline-silicon-solar-panel-55v-6w.html)**

### Solar panel, option 2:
- **[Soshine Mini Solar Panel](https://www.amazon.com/gp/product/B099RSLNZ4/ref=ox_sc_rp_title_rp_4?smid=&pf_rd_p=5d865192-d391-4c21-b512-33ef668eaf70&pf_rd_r=ZD34D17Q1Y9YK24T2YJF&pd_rd_wg=ZZ4WH&pd_rd_i=B099RSLNZ4&pd_rd_w=qxl9E&content-id=amzn1.sym.5d865192-d391-4c21-b512-33ef668eaf70&pd_rd_r=34bb9ed1-854c-4531-a427-4dfd883824a8&th=1)**
  USB Solar Panel Charger 5v 6w.

### Battery pack (enough with 4000mha, but...):
- **[3.7V 18650 10500mAh 1S3P Li-ion Battery](https://www.amazon.com/CASFROM-Compatible-Raspberry-Expansion-Simplehuman/dp/B0F6YQ5N89/ref=sr_1_3_sspa?crid=1Y3TGZSPH0CLH&dib=eyJ2IjoiMSJ9.2cC4wHjAOveuni3fPLgklI2anq8qZR3mahU_1NZ8Wfkr_xLWDXc3jyd4yNKjtAzIJgP6rkHCaVlKyeBhPLEavPd6ATVfCvVypiUZy9Fz5wVC7qCxgWrsOkE4iVT9EJH9b3zQEnJbewSdcbDe1_evJixsyR-V9SwzzXcvsmppd3Qv9EkH1Da1TYBWlWirT4vLtMP1MdHqve-rQfHhaF4oaLit2LzsnA4UtkWgzXZwiHsscuDUr50korjJxhCqK0t619teYui8PROMIDfy_CPHgxaTLrPr0RB3jxQCIlf-bRw.tuKJZKJZRW2r4_WaC515kUx-KhOzimWtvhVpHQ-kamo&dib_tag=se&keywords=lithium+battery+pack+18650&qid=1755907422&sprefix=lithium+battery+pack+18650%2Caps%2C228&sr=8-3-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)**

### Solar & Battery charger (powers the whole project):
- **[Adafruit charger](https://core-electronics.com.au/adafruit-universal-usb-dc-solar-lithium-ion-polymer-charger-bq24074.html)**

### Buck converters ( 2 needed. One lifts the voltage up tp 12v when sending, to avoid drops over the long cable, and the other lowers it, at the receiving end, to power the MCU safely):
- **[step up](https://core-electronics.com.au/12v-step-up-voltage-regulator-u3v16f12.html)**
- **[step down](https://core-electronics.com.au/adjustable-switching-power-supply-module-in-4v-35v-out-1-5v-30v-lm2596s.html)**

### Meshtastic node. Flashed with stock firmware:
- **[xiao-esp32s3](https://core-electronics.com.au/xiao-esp32s3-meshtastic-lora-wio-sx1262-wireless-communication-device.html)**

### Magnetometer sensor. Senses disturbances in the Earth’s magnetic field caused by large metal objects passing nearby, i.e. vehicles:
- **[MLX90393](https://www.digikey.com.au/en/products/detail/adafruit-industries-llc/4022/9924180)**

### MCU Co-processor. Does the reading, compares current azimuth against a smoothed average + trigger event if threshold crossed:
- **[esp32-s3-mini](https://core-electronics.com.au/esp32-s3-mini-development-board-retired.html)**

### Transreceiver (allows for long cable connectivity. Get powered by their nearest MCU's 3V3 regulators, respectively):
- **[RS485-TTL](https://core-electronics.com.au/2-channel-rs485-module-raspberry-pi-pico-sp3485-uart.html)**

### Weatherproof Connector. Insulates the ethernet cable into the box:
- **[Cable glands](https://core-electronics.com.au/waterproof-cable-gland-pg-7-size-black.html)**



---

## Existing infrastructure
- **[Solar Meshtastic Node](https://es.aliexpress.com/item/32908615100.html?spm=a2g0o.order_list.order_list_main.5.281f1802KYNsAG&gatewayAdapt=glo2esp)**


## Wiring Notes

Remote Sensor Node
------------------
    [Sensor]
       ↓  SPI
    MCU (ESP32-S3 Mini)
       ↓ UART (via jumper cable)
    [RS-485 Driver]
       ↓
    Long-Distance Link
      ETHERNET cable
    ------------------
    Twisted Pair (A/B, up to tens of meters)

Base/Mesh Node
--------------
    [RS-485 Receiver]
       ↓ UART (via jumper cable)
    XIAO ESP32-S3 (Meshtastic Node)
       ↓
    Serial Module (TEXTMSG)
       ↓
    Mesh Network Output

---

## Detection Logic (DRAFT)

- Sample magnetometer once per second  
- Compare current azimuth against a smoothed average  
- If deviation > threshold (e.g. **2.5°**) → trigger event  
- On trigger:  
  ```cpp
  MeshtasticDevice.sendText("Vehicle detected at...");
  ```
