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
      calMean_ += delta / calCount_;
      calM2_ += delta * (mag_uT - calMean_);

      return DetectEvent::Calibrating;
    }

    state_.baseline_uT = calMean_;

    float rawNoise = (calCount_ > 1)
      ? sqrtf(calM2_ / (calCount_ - 1))
      : MIN_CAL_NOISE_STD_UT;

    // Clamp calibration noise so one bad calibration cannot create a useless threshold.
    if (isnan(rawNoise) || rawNoise < MIN_CAL_NOISE_STD_UT) {
      state_.noiseStd_uT = MIN_CAL_NOISE_STD_UT;
    } else if (rawNoise > MAX_CAL_NOISE_STD_UT) {
      state_.noiseStd_uT = MAX_CAL_NOISE_STD_UT;
    } else {
      state_.noiseStd_uT = rawNoise;
    }

    float dyn = fmaxf(
      ABS_THRESHOLD_UT,
      K_SIGMA * state_.noiseStd_uT
    );

    // Hard cap for validation mode.
    state_.dynThresh_uT = fminf(dyn, MAX_DYNAMIC_THRESHOLD_UT);

    calDone_ = true;
    return DetectEvent::CalDone;
  }

  // --- 2) Tracking & detection ---
  float delta_uT = fabsf(mag_uT - state_.baseline_uT);

  // Slowly move baseline only when no event is active.
  if (!state_.eventActive) {
    state_.baseline_uT =
      (1.0f - BASELINE_ALPHA) * state_.baseline_uT
      + BASELINE_ALPHA * mag_uT;
  }

  const float highThresh = state_.dynThresh_uT;
  const float lowThresh  = state_.dynThresh_uT * HYSTERESIS_FRACTION;

  if (!state_.eventActive) {
    if (delta_uT >= highThresh) {
      if (++highCount_ >= N_CONSEC_HIGH) {
        state_.eventActive = true;
        eventStartMs_ = now_ms;
        highCount_ = 0;
        lowCount_  = 0;
        return DetectEvent::VehicleDetected;
      }
    } else {
      highCount_ = 0;
    }
  } else {
    if ((now_ms - eventStartMs_) < EVENT_HOLD_MS) {
      lowCount_ = 0;
      return DetectEvent::None;
    }

    if (delta_uT <= lowThresh) {
      if (++lowCount_ >= N_CONSEC_LOW) {
        state_.eventActive = false;
        lowCount_  = 0;
        highCount_ = 0;
        return DetectEvent::EventCleared;
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
           + String(" delta_uT=") + String(delta_uT, 3)
           + String(" active=") + String(state_.eventActive ? 1 : 0);
  return s;
}