// /include/vehicle_detector.h

#pragma once
#include <Arduino.h>

enum class DetectEvent {
  None,
  Calibrating,
  CalDone,
  VehicleDetected,   // ALARM
  EventCleared       // NO_ALARM
};

struct DetectorState {
  bool   eventActive   = false;
  float  baseline_uT   = NAN;
  float  noiseStd_uT   = NAN;
  float  dynThresh_uT  = NAN;
};

class VehicleDetector {
public:
  VehicleDetector() = default;

  // Call at your sample rate with |B| in microtesla
  DetectEvent update(float mag_uT, uint32_t now_ms);

  const DetectorState& state() const { return state_; }

  // Simple debug line for USB Serial only
  String bench(float mag_uT, float delta_uT) const;

private:
  // Calibration start time is set on first update()
  bool started_ = false;
  uint32_t startMs_ = 0;

  // Welford running stats during calibration
  uint32_t calCount_ = 0;
  float    calMean_  = 0.0f;
  float    calM2_    = 0.0f;

  bool     calDone_  = false;

  // Hysteresis counters
  uint8_t  highCount_ = 0;
  uint8_t  lowCount_  = 0;

  // Event timing (cooldown/hold logic)
  uint32_t eventStartMs_ = 0;

  DetectorState state_;
};
