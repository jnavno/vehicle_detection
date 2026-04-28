#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90393.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#include "config.h"
#include "vehicle_detector.h"
#include "forwarder.h"

// Waveshare S3 Zero onboard RGB LED is commonly GPIO21.
// If your board variant differs, change this pin.
Adafruit_NeoPixel pixel(1, 21, NEO_GRB + NEO_KHZ800);

Adafruit_MLX90393 mag;
VehicleDetector detector;

bool sensorConfigured = false;
uint32_t lastBenchMs = 0;

bool sensorBegin() {
  Wire.begin(MAG_I2C_SDA_PIN, MAG_I2C_SCL_PIN);
  Wire.setClock(100000);

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

bool readMagnitude(float& mag_uT) {
  if (!sensorConfigured) return false;

  float x = 0;
  float y = 0;
  float z = 0;

  if (!mag.readData(&x, &y, &z)) {
    return false;
  }

  mag_uT = sqrtf(x * x + y * y + z * z);
  return true;
}

void heartbeat() {
  static uint32_t last = 0;
  static bool on = false;

  if (millis() - last > 500) {
    last = millis();
    on = !on;

    pixel.setPixelColor(0, on ? pixel.Color(0, 0, 40) : pixel.Color(0, 0, 0));
    pixel.show();
  }
}

void sensorFailBlink() {
  pixel.setPixelColor(0, pixel.Color(50, 0, 0));
  pixel.show();
  delay(100);

  pixel.setPixelColor(0, pixel.Color(0, 0, 0));
  pixel.show();
  delay(100);
}

void setup() {
  pixel.begin();
  pixel.setBrightness(30);
  pixel.clear();
  pixel.show();

  Forward::begin();

  if (!sensorBegin()) {
    Serial.println(F("CRITICAL: Sensor Init Fail"));
    Forward::sendMeshText("ERR: MAG_INIT_FAIL");

    while (true) {
      Forward::poll();
      sensorFailBlink();
    }
  }

  Serial.println(F("System Online. Calibrating..."));
  Forward::sendMeshText("STATUS: CALIBRATING");
}

void loop() {
  heartbeat();

  // Always poll UART so any Heltec serial output is visible on USB debug.
  Forward::poll();

  // Always send heartbeat pings, even if the magnetometer path has issues.
  Forward::sendTestPingIfDue();

  float mag_uT = NAN;

  if (!readMagnitude(mag_uT)) {
    static uint32_t lastErr = 0;

    if (millis() - lastErr > 30000) {
      Serial.println(F("ERR: MAG_LOST"));
      Forward::sendMeshText("ERR: MAG_LOST");
      lastErr = millis();
    }

    delay(SAMPLE_PERIOD_MS);
    return;
  }

  const uint32_t now = millis();

  DetectEvent ev = detector.update(mag_uT, now);
  const DetectorState& st = detector.state();

  if (ev == DetectEvent::VehicleDetected ||
      ev == DetectEvent::EventCleared ||
      ev == DetectEvent::CalDone) {
    float delta = isnan(st.baseline_uT) ? 0.0f : fabsf(mag_uT - st.baseline_uT);
    Forward::sendEvent(ev, mag_uT, delta, st, now);
  }

  if (now - lastBenchMs > DEBUG_BENCH_PERIOD_MS) {
    float delta = isnan(st.baseline_uT) ? 0.0f : fabsf(mag_uT - st.baseline_uT);
    Serial.println(detector.bench(mag_uT, delta));
    lastBenchMs = now;
  }

  delay(SAMPLE_PERIOD_MS);
}