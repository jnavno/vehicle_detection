#pragma once
#include <Arduino.h>

/*
==========================================================
  PROJECT: Waveshare ESP32-S3 + MLX90393 Vehicle Detector
  OUTPUT MODE: UART TEXTMSG into RAK4630/RAK4631 Meshtastic
==========================================================

Wiring:
  Waveshare GPIO9 TX  -> RAK RXD1 / pin 15
  Waveshare GPIO8 RX  <- RAK TXD1 / pin 16   optional
  Waveshare GND       -> RAK GND

Current validation:
  Waveshare GPIO9 TX -> USB-TTL RX works.
  TTL receives HB_RAK at 38400 baud.
*/

// ========================================================
//  BOARD / SENSOR SETTINGS
// ========================================================

#define MAG_I2C_SDA_PIN   1
#define MAG_I2C_SCL_PIN   2

#define NEOPIXEL_PIN      21
#define NEOPIXEL_COUNT    1

// ========================================================
//  UART LINK TO RAK MESHTASTIC NODE
// ========================================================

#define ENABLE_LINK_UART        1
#define LINK_BAUD               38400

#define LINK_TX_PIN             9
#define LINK_RX_PIN             8

#define LINK_DEBUG_RX_ECHO      1

#define ENABLE_TEST_PING        1
#define TEST_PING_PERIOD_MS     10000

// ========================================================
//  DISABLE DETECTION GPIO OUTPUT
// ========================================================

#define ENABLE_DETECT_GPIO_OUT  0
#define DETECT_OUT_PIN          9
#define DETECT_OUT_ACTIVE_HIGH  1

// ========================================================
//  DETECTOR TUNING - BASIC VALIDATION MODE
// ========================================================

// Sampling
#define SAMPLE_PERIOD_MS        20

// Calibration
// Keep the sensor quiet during this time.
// If calibration includes a vehicle/magnet movement, noise estimate will be bad.
#define CAL_TIME_MS             8000

// Absolute minimum threshold.
#define ABS_THRESHOLD_UT        4.0f

// Dynamic threshold = K_SIGMA * noise.
// Lowered from 8 to 5 for validation.
#define K_SIGMA                 5.0f

// Hard cap so a noisy calibration cannot make detection impossible.
// Your bad run had dynThresh around 91 uT. This cap prevents that.
#define MAX_DYNAMIC_THRESHOLD_UT 8.0f

// If calibration noise is insane, clamp it before computing threshold.
#define MAX_CAL_NOISE_STD_UT    2.0f

// If calibration somehow produces near-zero noise, use this floor.
#define MIN_CAL_NOISE_STD_UT    0.25f

// Trigger / clear timing
// Lower values for bench validation.
#define N_CONSEC_HIGH           3
#define N_CONSEC_LOW            8

#define EVENT_HOLD_MS           3000
#define HYSTERESIS_FRACTION     0.40f

// Baseline tracking
// Slow enough not to eat events quickly.
#define BASELINE_ALPHA          0.0005f

// ========================================================
//  USB DEBUG
// ========================================================

#define DEBUG_BENCH_PERIOD_MS   2000

#define TUNING_MODE_CSV         1
#define CSV_PERIOD_MS           100

#define ENABLE_STATUS_LOGS      1