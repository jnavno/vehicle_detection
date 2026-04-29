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
  bool   eventActive   = false;

  // Magnitude of baseline vector, kept for readable logs.
  float  baseline_uT   = NAN;

  // 3-axis baseline.
  float  baselineX_uT  = NAN;
  float  baselineY_uT  = NAN;
  float  baselineZ_uT  = NAN;

  float  noiseStd_uT   = NAN;
  float  dynThresh_uT  = NAN;

  // Last vector delta.
  float  delta_uT      = NAN;
  float  peakDelta_uT  = 0.0f;
};

class VehicleDetector {
public:
  VehicleDetector() = default;

  // Preferred 3-axis update.
  DetectEvent update(float x_uT, float y_uT, float z_uT, uint32_t now_ms);

  const DetectorState& state() const { return state_; }

  String bench(float mag_uT, float delta_uT) const;

private:
  bool started_ = false;
  uint32_t startMs_ = 0;

  uint32_t calCount_ = 0;

  // Welford per axis.
  float calMeanX_ = 0.0f;
  float calMeanY_ = 0.0f;
  float calMeanZ_ = 0.0f;

  float calM2X_ = 0.0f;
  float calM2Y_ = 0.0f;
  float calM2Z_ = 0.0f;

  bool calDone_ = false;

  uint8_t highCount_ = 0;
  uint8_t lowCount_  = 0;

  uint32_t eventStartMs_ = 0;

  DetectorState state_;

  void rebaseline(float x_uT, float y_uT, float z_uT);
};