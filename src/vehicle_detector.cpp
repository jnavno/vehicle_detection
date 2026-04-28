// /src/vehicle_detector.cpp

#include "vehicle_detector.h"
#include "config.h"
#include <math.h>

DetectEvent VehicleDetector::update(float mag_uT, uint32_t now_ms) {
  if (!started_) {
    startMs_ = now_ms;
    started_ = true;
  }

  // --- 1) Calibration phase ---
  if (!calDone_ || isnan(state_.baseline_uT)) {
    if ((now_ms - startMs_) < CAL_TIME_MS) {
      calCount_++;
      float delta = mag_uT - calMean_;
      calMean_   += delta / calCount_;
      calM2_     += delta * (mag_uT - calMean_);
      return DetectEvent::Calibrating;
    }

    state_.baseline_uT = calMean_;
    state_.noiseStd_uT = (calCount_ > 1)
      ? sqrtf(calM2_ / (calCount_ - 1))
      : 0.5f;

    state_.dynThresh_uT = fmaxf(
      ABS_THRESHOLD_UT,
      K_SIGMA * state_.noiseStd_uT
    );

    calDone_ = true;
    return DetectEvent::CalDone;
  }

  // --- 2) Tracking & detection ---
  float delta_uT = fabsf(mag_uT - state_.baseline_uT);

  // Slowly move baseline when we are *not* in an active event
  if (!state_.eventActive) {
    state_.baseline_uT =
      (1.0f - BASELINE_ALPHA) * state_.baseline_uT
      + BASELINE_ALPHA * mag_uT;
  }

  const float highThresh = state_.dynThresh_uT;
  const float lowThresh  = state_.dynThresh_uT * HYSTERESIS_FRACTION;

  if (!state_.eventActive) {
    // look for enough consecutive “high” samples to trigger alarm
    if (delta_uT >= highThresh) {
      if (++highCount_ >= N_CONSEC_HIGH) {
        state_.eventActive = true;
        eventStartMs_ = now_ms;
        highCount_ = 0;
        lowCount_  = 0;
        return DetectEvent::VehicleDetected;  // ALARM
      }
    } else {
      highCount_ = 0;
    }
  } else {
    // in an active event: enforce a minimum hold time before allowing CLEAR
    if ((now_ms - eventStartMs_) < EVENT_HOLD_MS) {
      lowCount_ = 0;
      return DetectEvent::None;
    }

    // After hold: wait for enough “low” samples to clear
    if (delta_uT <= lowThresh) {
      if (++lowCount_ >= N_CONSEC_LOW) {
        state_.eventActive = false;
        lowCount_  = 0;
        highCount_ = 0;
        return DetectEvent::EventCleared;     // NO_ALARM
      }
    } else {
      lowCount_ = 0;
    }
  }

  return DetectEvent::None;
}

String VehicleDetector::bench(float mag_uT, float delta_uT) const {
  String s = "[BENCH]"
           + String(" baseline_uT=") + String(state_.baseline_uT, 3)
           + String(" noiseStd_uT=") + String(state_.noiseStd_uT, 3)
           + String(" dynThresh_uT=") + String(state_.dynThresh_uT, 3)
           + String(" mag_uT=") + String(mag_uT, 3)
           + String(" delta_uT=") + String(delta_uT, 3);
  return s;
}
