// config.h
#pragma once

#include <Arduino.h>

// ================== BOARD & WIRING ==================
// Waveshare ESP32-S3 Zero + MLX90393 magnetometer (I2C)
// + RS-485 link to Meshtastic node (via TTL<->RS485 transceivers)

// ---- I2C for MLX90393 magnetometer ----
// Choose any free GPIOs and wire SDA/SCL to the MLX90393.
#define MAG_I2C_SDA_PIN   1   // set to your actual SDA pin
#define MAG_I2C_SCL_PIN   2   // set to your actual SCL pin

// ------------------ UART LINK (TTL side) ------------------
// This is the ESP32 UART that goes into the *TTL pins* of the RS-485 transceiver.
// Wiring on ESP32 side (TTL):
//   ESP32 TX  --> Transceiver RXD    [input to the transceiver]
//   ESP32 RX  <-- Transceiver TXD    [output from the transceiver]
//   ESP32 GND <--> Transceiver GND
//   ESP32 3V3/5V --> Transceiver VCC (controlled via voltage regulator)
//
// Then the long-distance bus wiring (RS-485 side):
//   Transceiver A  <-->  Transceiver A
//   Transceiver B  <-->  Transceiver B
//   GND/reference <--> GND/reference for long runs/noise immunity
//
#define ENABLE_LINK_UART  1
#define LINK_BAUD         115200

// This is the ESP32 UART that goes into the *TTL pins* of the RS-485 transceiver.
// Wiring on ESP32 side (TTL):
#define LINK_TX_PIN       9   // ESP32-S3 TX -> transceiver RXD
#define LINK_RX_PIN       8   // ESP32-S3 RX -> transceiver TXD

//       Share GND between ESP32 and RS485 module.

// ================== SAMPLING & DETECTION ==================

// Sampling: 50 Hz (20 ms)
#define SAMPLE_PERIOD_MS        20

// Calibration time at startup: 5 seconds of “quiet” environment
#define CAL_TIME_MS             5000

// Threshold / detector tuning parameters
#define ABS_THRESHOLD_UT        2.0f     // minimum delta in microtesla
#define K_SIGMA                 5.0f     // dynamic threshold factor
#define N_CONSEC_HIGH           5        // samples to assert alarm
#define N_CONSEC_LOW            20       // samples to clear alarm
#define HYSTERESIS_FRACTION     0.5f     // lowThresh = highThresh * this
#define BASELINE_ALPHA          0.0015f  // slow baseline drift tracking
#define EVENT_HOLD_MS           3000    // 3 seconds. Try 2000–5000 in real tests

// Debug print period for USB Serial (ms)
#define DEBUG_BENCH_PERIOD_MS   2000
#define STATUS_LED_PIN  21
