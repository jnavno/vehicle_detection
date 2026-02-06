// main.cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90393.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#include "config.h"
#include "vehicle_detector.h"
#include "forwarder.h"

// =====================================================
// 1) HARDWARE: NeoPixel + MLX90393
// =====================================================

// Waveshare ESP32-S3 Zero NeoPixel is on GPIO21
Adafruit_NeoPixel pixel(1, 21, NEO_GRB + NEO_KHZ800);

Adafruit_MLX90393 mag;
static bool sensorConfigured = false;

static bool sensorBegin() {
  Wire.begin(MAG_I2C_SDA_PIN, MAG_I2C_SCL_PIN);

  if (!mag.begin_I2C(MLX90393_DEFAULT_ADDR, &Wire)) {
    return false;
  }

  mag.setGain(MLX90393_GAIN_1X);
  mag.setResolution(MLX90393_X, MLX90393_RES_17);
  mag.setResolution(MLX90393_Y, MLX90393_RES_17);
  mag.setResolution(MLX90393_Z, MLX90393_RES_16);
  mag.setOversampling(MLX90393_OSR_3);
  mag.setFilter(MLX90393_FILTER_5);

  sensorConfigured = true;
  return true;
}

static bool readXYZ(float& x, float& y, float& z) {
  if (!sensorConfigured) return false;
  return mag.readData(&x, &y, &z);
}

static bool readMagnitude(float& mag_uT) {
  float x, y, z;
  if (!readXYZ(x, y, z)) return false;
  mag_uT = sqrtf(x * x + y * y + z * z);
  return true;
}

// =====================================================
// 2) APP STATE
// =====================================================

VehicleDetector detector;
static uint32_t lastBenchMs = 0;

// NeoPixel heartbeat (blue blink)
static void heartbeat() {
  static uint32_t last = 0;
  static bool on = false;

  if (millis() - last > 500) {
    last = millis();
    on = !on;
    pixel.setPixelColor(0, on ? pixel.Color(0, 0, 40) : pixel.Color(0, 0, 0));
    pixel.show();
  }
}

void setup() {
  // NeoPixel init
  pixel.begin();
  pixel.setBrightness(30);
  pixel.setPixelColor(0, pixel.Color(0, 0, 0));
  pixel.show();

  // Optional extra status pin (only if you defined it)
#ifdef STATUS_LED_PIN
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
#endif

  Forward::begin();
  Serial.println(F("BOOT: sketch started"));
  Serial.println(F("BOOT: entering sensorBegin()..."));

  if (!sensorBegin()) {
    Serial.println(F("ERROR: MLX90393 not found (check wiring, pins, power)."));

    // Visible failure mode: blink NeoPixel red fast forever
    while (true) {
      pixel.setPixelColor(0, pixel.Color(40, 0, 0));
      pixel.show();
      delay(150);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      pixel.show();
      delay(150);
    }
  }

  Serial.println(F("Sensor OK. Calibrating..."));
#if !TUNING_MODE_CSV
  if (!isnan(delta_uT) && (now - lastBenchMs > DEBUG_BENCH_PERIOD_MS)) {
    Forward::sendDebug(detector.bench(mag_uT, delta_uT));
    lastBenchMs = now;
  }
#endif


}

void loop() {
  heartbeat();
  Forward::sendTestPingIfDue();

  float mag_uT;
  if (!readMagnitude(mag_uT)) {
    Forward::sendDebug(F("Sensor read fail"));
    delay(SAMPLE_PERIOD_MS);
    return;
  }

  uint32_t now = millis();
  DetectEvent ev = detector.update(mag_uT, now);
  const auto& st = detector.state();

  float delta_uT = (isnan(st.baseline_uT)) ? NAN : fabsf(mag_uT - st.baseline_uT);
  #if TUNING_MODE_CSV
  static uint32_t lastCsv = 0;
  if (now - lastCsv >= CSV_PERIOD_MS && !isnan(delta_uT)) {
    lastCsv = now;

    Serial.print(now);
    Serial.print(',');
    Serial.print(mag_uT, 3);
    Serial.print(',');
    Serial.print(delta_uT, 3);
    Serial.print(',');
    Serial.print(st.baseline_uT, 3);
    Serial.print(',');
    Serial.print(st.dynThresh_uT, 3);
    Serial.print(',');
    Serial.println(st.eventActive ? 1 : 0);
  }
#endif

  // Send events (USB detailed + mesh short)
  if (ev == DetectEvent::VehicleDetected ||
      ev == DetectEvent::EventCleared   ||
      ev == DetectEvent::CalDone) {
    Forward::sendEvent(ev, mag_uT, delta_uT, st, now);
  }

  // Periodic debug bench line over USB only
  if (!isnan(delta_uT) && (now - lastBenchMs > DEBUG_BENCH_PERIOD_MS)) {
    Forward::sendDebug(detector.bench(mag_uT, delta_uT));
    lastBenchMs = now;
  }

  delay(SAMPLE_PERIOD_MS);
}
