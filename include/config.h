#pragma once
#include <Arduino.h>

// --------------------
// I2C for MLX90393
// --------------------
#define MAG_I2C_SDA_PIN   1
#define MAG_I2C_SCL_PIN   2

// --------------------
// UART link to Heltec V4 / Wireless Tracker Meshtastic node
// --------------------
//
// Wiring:
//   Waveshare GPIO9 TX  ---> Heltec GPIO44 RX
//   Waveshare GPIO8 RX  <--- Heltec GPIO43 TX
//   GND                 --- GND
//
// Heltec Meshtastic Serial Module:
//   enabled: true
//   mode: TEXTMSG
//   rx_gpio: 44
//   tx_gpio: 43
//   baud: 115200
//
#define ENABLE_LINK_UART  1
#define LINK_BAUD         38400
#define LINK_TX_PIN       9
#define LINK_RX_PIN       8

// Send debug heartbeat text to Meshtastic serial
#define TEST_PING_PERIOD_MS     15000

// Print UART RX bytes from Heltec to USB serial.
// Useful for seeing whether Heltec is talking back.
#define LINK_DEBUG_RX_ECHO      1

// --------------------
// Detection tuning
// --------------------
#define SAMPLE_PERIOD_MS        20
#define CAL_TIME_MS             5000
#define ABS_THRESHOLD_UT        2.5f
#define K_SIGMA                 6.0f
#define N_CONSEC_HIGH           5
#define N_CONSEC_LOW            15
#define EVENT_HOLD_MS           3000
#define HYSTERESIS_FRACTION     0.4f
#define BASELINE_ALPHA          0.001f

// --------------------
// USB debug
// --------------------
#define DEBUG_BENCH_PERIOD_MS   5000