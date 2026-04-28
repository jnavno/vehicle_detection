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

RAK Meshtastic Serial Module:
  Enabled: ON
  Mode: TEXTMSG
  RXD: 15
  TXD: 16
  Baud: BAUD_38400
  Timeout: 1000
  Echo: OFF
  Override console serial port: OFF
  GPS mode: NOT_PRESENT / disabled

Message format:
  ASCII text + CRLF
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

// Waveshare side pins
#define LINK_TX_PIN             9
#define LINK_RX_PIN             8

// Echo any bytes received from RAK TX back to Waveshare USB log.
// Useful only if you wire RAK TXD1/pin16 -> Waveshare GPIO8.
#define LINK_DEBUG_RX_ECHO      1

// Send periodic UART heartbeat.
#define ENABLE_TEST_PING        1
#define TEST_PING_PERIOD_MS     10000

// ========================================================
//  DISABLE DETECTION GPIO OUTPUT
// ========================================================

#define ENABLE_DETECT_GPIO_OUT  0
#define DETECT_OUT_PIN          9
#define DETECT_OUT_ACTIVE_HIGH  1

// ========================================================
//  DETECTOR TUNING
// ========================================================

#define SAMPLE_PERIOD_MS        20
#define CAL_TIME_MS             10000

#define ABS_THRESHOLD_UT        4.0f
#define K_SIGMA                 8.0f

// Lowered for bring-up. Increase later if too sensitive.
#define N_CONSEC_HIGH           3
#define N_CONSEC_LOW            10

#define EVENT_HOLD_MS           5000
#define HYSTERESIS_FRACTION     0.35f
#define BASELINE_ALPHA          0.0006f

// ========================================================
//  USB DEBUG
// ========================================================

#define DEBUG_BENCH_PERIOD_MS   2000

#define TUNING_MODE_CSV         1
#define CSV_PERIOD_MS           100

#define ENABLE_STATUS_LOGS      1