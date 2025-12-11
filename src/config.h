// config.h
#pragma once

#include <Arduino.h>

// ================== BOARD & WIRING ==================
// Waveshare ESP32-S3 Mini / Zero + SP3485 RS485 module

// ---- I2C for MLX90393 magnetometer ----
// Choose any free GPIOs and wire SDA/SCL to the MLX90393.
// These are example pins: CHANGE TO MATCH YOUR WIRING.
#define MAG_I2C_SDA_PIN   8   // TODO: set to your actual SDA pin
#define MAG_I2C_SCL_PIN   9   // TODO: set to your actual SCL pin

// ---- UART for RS485 (SP3485) ----
// Serial1 will talk to the RS485 module (DI/RO).
// Choose any free GPIOs and wire them:
//   RS485 DI  <- ESP32 TX
//   RS485 RO  -> ESP32 RX
#define ENABLE_RS485_UART  1
#define RS485_BAUD         115200

// Example pins – adjust to your real wiring:
#define RS485_TX_PIN       17   // ESP32-S3 TX to SP3485 DI
#define RS485_RX_PIN       18   // ESP32-S3 RX from SP3485 RO

// NOTE: Tie SP3485 RE and DE pins HIGH (to 3V3) on the board,
//       so the transceiver is always enabled.
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

// Debug print period for USB Serial (ms)
#define DEBUG_BENCH_PERIOD_MS   2000
