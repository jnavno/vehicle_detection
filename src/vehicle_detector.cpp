#include "vehicle_detector.h"
#include "config.h"
#include <math.h>

static float vecMag(float x, float y, float z) {
  return sqrtf(x * x + y * y + z * z);
}

static float vecDelta(float x, float y, float z, float bx, float by, float bz) {
  float dx = x - bx;
  float dy = y - by;
  float dz = z - bz;
  return sqrtf(dx * dx + dy * dy + dz * dz);
}

void VehicleDetector::rebaseline(float x_uT, float y_uT, float z_uT) {
  state_.baselineX_uT = x_uT;
  state_.baselineY_uT = y_uT;
  state_.baselineZ_uT = z_uT;
  state_.baseline_uT = vecMag(x_uT, y_uT, z_uT);

  state_.delta_uT = 0.0f;
  state_.peakDelta_uT = 0.0f;

  highCount_ = 0;
  lowCount_ = 0;
}

DetectEvent VehicleDetector::update(float x_uT, float y_uT, float z_uT, uint32_t now_ms) {
  if (!started_) {
    startMs_ = now_ms;
    started_ = true;
  }

  // --- 1) Calibration phase ---
  if (!calDone_ || isnan(state_.baseline_uT)) {
    if ((now_ms - startMs_) < CAL_TIME_MS) {
      calCount_++;

      float dx = x_uT - calMeanX_;
      calMeanX_ += dx / calCount_;
      calM2X_ += dx * (x_uT - calMeanX_);

      float dy = y_uT - calMeanY_;
      calMeanY_ += dy / calCount_;
      calM2Y_ += dy * (y_uT - calMeanY_);

      float dz = z_uT - calMeanZ_;
      calMeanZ_ += dz / calCount_;
      calM2Z_ += dz * (z_uT - calMeanZ_);

      return DetectEvent::Calibrating;
    }

    rebaseline(calMeanX_, calMeanY_, calMeanZ_);

    float rawNoise = MIN_CAL_NOISE_STD_UT;

    if (calCount_ > 1) {
      float varX = calM2X_ / (calCount_ - 1);
      float varY = calM2Y_ / (calCount_ - 1);
      float varZ = calM2Z_ / (calCount_ - 1);

      // Approx vector noise.
      rawNoise = sqrtf(varX + varY + varZ);
    }

    if (isnan(rawNoise) || rawNoise < MIN_CAL_NOISE_STD_UT) {
      state_.noiseStd_uT = MIN_CAL_NOISE_STD_UT;
    } else if (rawNoise > MAX_CAL_NOISE_STD_UT) {
      state_.noiseStd_uT = MAX_CAL_NOISE_STD_UT;
    } else {
      state_.noiseStd_uT = rawNoise;
    }

    float dyn = fmaxf(ABS_THRESHOLD_UT, K_SIGMA * state_.noiseStd_uT);
    state_.dynThresh_uT = fminf(dyn, MAX_DYNAMIC_THRESHOLD_UT);

    calDone_ = true;
    return DetectEvent::CalDone;
  }

  // --- 2) Tracking & detection ---
  float delta_uT = vecDelta(
    x_uT, y_uT, z_uT,
    state_.baselineX_uT,
    state_.baselineY_uT,
    state_.baselineZ_uT
  );

  state_.delta_uT = delta_uT;

  if (delta_uT > state_.peakDelta_uT) {
    state_.peakDelta_uT = delta_uT;
  }

  // Slowly move baseline only when no event is active.
  if (!state_.eventActive) {
    state_.baselineX_uT = (1.0f - BASELINE_ALPHA) * state_.baselineX_uT + BASELINE_ALPHA * x_uT;
    state_.baselineY_uT = (1.0f - BASELINE_ALPHA) * state_.baselineY_uT + BASELINE_ALPHA * y_uT;
    state_.baselineZ_uT = (1.0f - BASELINE_ALPHA) * state_.baselineZ_uT + BASELINE_ALPHA * z_uT;
    state_.baseline_uT = vecMag(state_.baselineX_uT, state_.baselineY_uT, state_.baselineZ_uT);
  }

  const float highThresh = state_.dynThresh_uT;
  const float lowThresh  = state_.dynThresh_uT * HYSTERESIS_FRACTION;

  if (!state_.eventActive) {
    if (delta_uT >= highThresh) {
      if (++highCount_ >= N_CONSEC_HIGH) {
        state_.eventActive = true;
        state_.peakDelta_uT = delta_uT;
        eventStartMs_ = now_ms;
        highCount_ = 0;
        lowCount_ = 0;
        return DetectEvent::VehicleDetected;
      }
    } else {
      highCount_ = 0;
    }
  } else {
    uint32_t activeMs = now_ms - eventStartMs_;

#if EVENT_STUCK_REBASE_MS > 0
    // Desktop validation safety: don't stay latched forever.
    if (activeMs >= EVENT_STUCK_REBASE_MS) {
      state_.eventActive = false;
      rebaseline(x_uT, y_uT, z_uT);
      return DetectEvent::EventCleared;
    }
#endif

    if (activeMs < EVENT_HOLD_MS) {
      lowCount_ = 0;
      return DetectEvent::None;
    }

    if (delta_uT <= lowThresh) {
      if (++lowCount_ >= N_CONSEC_LOW) {
        state_.eventActive = false;
        state_.peakDelta_uT = 0.0f;
        lowCount_ = 0;
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
           + String(" deltaVec_uT=") + String(delta_uT, 3)
           + String(" peak_uT=") + String(state_.peakDelta_uT, 3)
           + String(" active=") + String(state_.eventActive ? 1 : 0);
  return s;
}