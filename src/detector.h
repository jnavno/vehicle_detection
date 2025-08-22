#pragma once
#include <Arduino.h>

enum class DetectEvent {
  None,
  Calibrating,
  CalDone,
  VehicleDetected,
  EventCleared
};

struct DetectorState {
  bool   eventActive = false;
  float  baseline_uT = NAN;
  float  noiseStd_uT = NAN;
  float  dynThresh_uT = NAN;
};

class VehicleDetector {
public:
  VehicleDetector();

  // Call every sample with (magnitude in uT, millis())
  DetectEvent update(float mag_uT, uint32_t now_ms);

  // Accessors
  const DetectorState& state() const { return st_; }

  // Optional: get a quick benchmark line
  String bench(float mag_uT, float delta_uT) const;

private:
  // Welford for calibration
  uint32_t calCount_ = 0;
  float    calMean_  = 0.0f;
  float    calM2_    = 0.0f;

  // phase / timing
  uint32_t startMs_ = 0;
  bool     calDone_ = false;

  // hysteresis counters
  uint8_t highCount_ = 0;
  uint8_t lowCount_  = 0;

  DetectorState st_;
};
