#include <Arduino.h>
#include "config.h"
#include "sensor_mlx90393.h"
#include "detector.h"
#include "forwarder.h"

MLX90393Sensor sensor;
VehicleDetector detector;

static uint32_t lastBenchMs = 0;

void setup() {
  Serial.begin(115200);
  // For USB CDC on ESP32-S3, this helps when tethered. Remove if you run headless.
  while (!Serial) { delay(10); }

  Serial.println(F("\nMLX90393 Vehicle-Detect (Heltec V3)"));
  Serial.println(F("Init sensor..."));
  if (!sensor.begin()) {
    Serial.println(F("ERROR: MLX90393 not found (check wiring, address, power)."));
    while (1) delay(10);
  }
  Serial.println(F("Sensor OK. Starting..."));

  Forward::begin();
}

void loop() {
  float mag;
  if (!sensor.readMagnitude(mag)) {
    Serial.println(F("read fail"));
    delay(SAMPLE_PERIOD_MS);
    return;
  }

  uint32_t now = millis();
  DetectEvent ev = detector.update(mag, now);

  const auto& st = detector.state();
  float delta_uT = (isnan(st.baseline_uT)) ? NAN : fabsf(mag - st.baseline_uT);

  // status / event reporting
  if (ev == DetectEvent::Calibrating) {
    // quiet during calibration, but you could blink an LED here if desired
  } else if (ev != DetectEvent::None) {
    Forward::sendEvent(ev, mag, delta_uT, st.baseline_uT, st.dynThresh_uT, now);
  }

  // periodic benchmark line
  if (now - lastBenchMs > DEBUG_BENCH_PERIOD_MS && !isnan(delta_uT)) {
    Forward::sendLine(detector.bench(mag, delta_uT));
    lastBenchMs = now;
  }

  delay(SAMPLE_PERIOD_MS);
}
