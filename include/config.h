#pragma once
#include <Arduino.h>

/*
==========================================================
  PROJECT: Waveshare ESP32-S3 + MLX90393 Vehicle Detector
  OUTPUT MODE: UART TEXTMSG into RAK4630/RAK4631 Meshtastic
==========================================================

Validated:
  Waveshare GPIO9 TX -> USB-TTL RX receives:
    HB_RAK
    VEH_ALARM
    VEH_CLEAR

Next:
  Waveshare GPIO9 TX -> RAK RXD1 / pin 15
  Waveshare GND      -> RAK GND
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
//  DETECTOR TUNING - DESKTOP VALIDATION MODE
// ========================================================

// Sampling
#define SAMPLE_PERIOD_MS        20

// Keep sensor quiet during calibration.
// Do not pass metal over the sensor until VEH_READY.
#define CAL_TIME_MS             8000

// Detection is now based on 3-axis vector delta:
// delta = sqrt((x-baselineX)^2 + (y-baselineY)^2 + (z-baselineZ)^2)

// Minimum threshold in uT.
#define ABS_THRESHOLD_UT        3.0f

// Dynamic threshold = K_SIGMA * calibration noise.
#define K_SIGMA                 5.0f

// Clamp threshold so bad calibration cannot make detection blind.
#define MAX_DYNAMIC_THRESHOLD_UT 8.0f

// Clamp calibration noise.
#define MAX_CAL_NOISE_STD_UT    2.0f
#define MIN_CAL_NOISE_STD_UT    0.20f

// Fast desktop validation.
// Increase later for field deployment if needed.
#define N_CONSEC_HIGH           2
#define N_CONSEC_LOW            8

// Minimum time an event remains active before clear is allowed.
#define EVENT_HOLD_MS           1500

// Desktop safety: if an event stays active too long, force clear and rebaseline.
// This prevents the detector from getting stuck after a bad calibration or metal left nearby.
// For deployment, consider setting this to 0.
#define EVENT_STUCK_REBASE_MS   15000

// CLEAR threshold = high threshold * this fraction.
#define HYSTERESIS_FRACTION     0.70f

// Baseline tracking when no event is active.
#define BASELINE_ALPHA          0.0005f

// ========================================================
//  USB DEBUG / LOGGING CONTROLS
// ========================================================

// Main debug switch.
// 1 = verbose desktop/bench logs.
// 0 = quiet field mode.
#define ENABLE_USB_DEBUG        1

// CSV stream over USB.
// Useful for tuning, noisy for deployment.
#define TUNING_MODE_CSV         1
#define CSV_PERIOD_MS           100

// Periodic [BENCH] lines over USB.
#define ENABLE_BENCH_LOGS       1
#define DEBUG_BENCH_PERIOD_MS   2000

// Boot/config/status logs over USB.
#define ENABLE_STATUS_LOGS      1

// Echo incoming bytes from RAK TX to Waveshare USB.
// Only useful if RAK TXD1 pin16 -> Waveshare GPIO8 is wired.
#define LINK_DEBUG_RX_ECHO      0