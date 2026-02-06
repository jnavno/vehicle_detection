// config.h
#pragma once
#include <Arduino.h>

/*
==========================================================
  PROJECT: Waveshare ESP32-S3 + MLX90393 Vehicle Detector
==========================================================

HOW TO USE THIS FILE TO TUNE THE DETECTOR:
- To tune detection behavior, we'll ONLY edit the section:
      "TUNING ONLY (SAFE TO EDIT)"
- Do NOT change pins/serial/I2C unless wiring changed.
- If something stops working after edits, revert this file first.
- Best to tune in small steps and test 1 change at a time.

*/


// ========================================================
//  WIRING / BOARD SETTINGS
// ========================================================

// ---- I2C for MLX90393 magnetometer ----
// These must match the physical wiring.
#define MAG_I2C_SDA_PIN   1
#define MAG_I2C_SCL_PIN   2

// ---- NeoPixel (Waveshare ESP32-S3 Zero onboard LED) ----
// On this board, the onboard RGB LED is a WS2812 on GPIO21.
// We use it for debugging/heartbeat only at this stage.
// No need to change unless using an external LED.
#define NEOPIXEL_PIN      21
#define NEOPIXEL_COUNT    1

// ---- UART LINK to RS-485 transceiver (TTL side) ----
// ESP32 TX -> Transceiver RXD
// ESP32 RX <- Transceiver TXD
#define ENABLE_LINK_UART  1
#define LINK_BAUD         115200
#define LINK_TX_PIN       9 //into 34 RX meshtastic heltec v3
#define LINK_RX_PIN       8 //into 33 TX meshtastic heltec v3

// Optional extra simple GPIO LED (only if using an external LED).
// If no discrete LED, leave this commented out.
// #define STATUS_LED_PIN  10



// ========================================================
//  TUNING ONLY (SAFE TO EDIT)
// ========================================================
/*
---------------------------
  What will changing these values do
---------------------------

This detector works like this:
1) CALIBRATION: for CAL_TIME_MS, we measure "quiet" magnetic field |B|
   and compute a baseline + noise level.
2) DETECTION: we compute delta = abs(|B| - baseline)
   If delta stays high long enough, we trigger ALARM.
   If delta stays low long enough (after a hold time), we CLEAR.

With this, we can tune:
- How sensitive it is (thresholds)
- How long it must stay high/low (durations)
- How fast baseline is allowed to drift (baseline tracking)

Let's start with small changes.
*/


// -------------------- Sampling --------------------
// How often we read the sensor.
// 20ms = 50 Hz. If we go faster, noise may increase.
#define SAMPLE_PERIOD_MS        20


// -------------------- Calibration --------------------
// Calibration requires a "quiet" period at boot.
// Increase if your environment is noisy at startup.
#define CAL_TIME_MS             10000


// -----PRIORITY----- Sensitivity thresholds --------------------
// Absolute minimum delta in microtesla (uT) to consider "something changed"
// Raise to reduce false positives.
#define ABS_THRESHOLD_UT        4.0f

// Dynamic threshold = max(ABS_THRESHOLD_UT, K_SIGMA * noiseStd)
// Raise K_SIGMA to reduce false positives.
// Lower K_SIGMA to increase sensitivity.
#define K_SIGMA                 8.0f


// ------PRIORITY----- Trigger / Clear timing --------------------
// How many consecutive "high delta" samples before ALARM.
// At 50 Hz: 5 samples = 0.10s, 20 samples = 0.40s.
#define N_CONSEC_HIGH           25

// How many consecutive "low delta" samples before CLEAR (after hold time).
// At 50 Hz: 20 samples = 0.40s, 50 samples = 1.00s.
#define N_CONSEC_LOW            75

// ----PRIORITY----- Anti-spam hold time --------------------
// After ALARM triggers, we hold the event for this minimum time.
// Prevents ALARM/CLEAR chatter with slow-moving vehicles.
#define EVENT_HOLD_MS           5000

// -------------------- Hysteresis --------------------
// CLEAR threshold is lower than ALARM threshold:
// lowThresh = highThresh * HYSTERESIS_FRACTION
// Smaller fraction = harder to clear (more stable, less chatter).
#define HYSTERESIS_FRACTION     0.35f


// -------------------- Baseline tracking --------------------
// Baseline slowly adapts when no event is active.
// Smaller alpha = baseline changes slower (more stable but less adaptive).
// Larger alpha = baseline adapts faster (can help long-term drift, but risks
// eating real events if too large).
#define BASELINE_ALPHA          0.0006f


// -------------------- Debug output --------------------
// How often we print the [BENCH] line over USB serial
#define DEBUG_BENCH_PERIOD_MS   2000


// ---------- Field test features ----------
#define ENABLE_TEST_PING       1
#define TEST_PING_PERIOD_MS    30000  // 30s

// CSV logging over USB (NOT over mesh)
#define TUNING_MODE_CSV        1
#define CSV_PERIOD_MS          100     // 10 Hz CSV so it’s readable



// ========================================================
//  ADVANCED TUNING (OPTIONAL, ONLY AFTER BASIC WORKS)
// ========================================================
/*
If we later want "levels" (METAL vs VEHICLE), we can add something like:

#define VEHICLE_THRESHOLD_UT  10.0f

And in the forwarder, send:
- "METAL d=..." for small deltas
- "VEH ALARM d=..." for large deltas
*/
