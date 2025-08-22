#pragma once

// ── Heltec WiFi LoRa 32 V3 (ESP32-S3) ─────────────────────────────
// I²C pins used by Heltec V3:
#define I2C_SDA_PIN 41
#define I2C_SCL_PIN 42

// ── Sampling & calibration ────────────────────────────────────────
#define SAMPLE_PERIOD_MS        20      // ~50 Hz
#define CAL_TIME_MS             5000    // 5 s initial ambient learn

// ── Thresholds / detector tuning ──────────────────────────────────
#define ABS_THRESHOLD_UT        2.0f    // absolute floor in microtesla
#define K_SIGMA                 5.0f    // dynamic: K * noiseStd
#define N_CONSEC_HIGH           5       // samples above threshold to assert
#define N_CONSEC_LOW            20      // samples below low threshold to clear
#define HYSTERESIS_FRACTION     0.5f    // lowThresh = highThresh * this
#define BASELINE_ALPHA          0.0015f // slow drift tracking (smaller = slower)

// ── Debug / status printing ───────────────────────────────────────
#define DEBUG_BENCH_PERIOD_MS   2000

// ──  Secondary UART forwarding (to another MCU/Meshtastic) ─
// Set to 1 to enable Serial1 output and define pins
#define ENABLE_UART1            0
#define UART1_BAUD              115200
// CHANGE THESE PINS TO whatever WIRING if we enable UART1:
#define UART1_TX_PIN            17
#define UART1_RX_PIN            18
