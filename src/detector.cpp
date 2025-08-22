#include "detector.h"
#include "config.h"
#include <math.h>

VehicleDetector::VehicleDetector() {
  startMs_ = millis();
}

String VehicleDetector::bench(float mag_uT, float delta_uT) const {
  String s = "[BENCH] baseline_uT=" + String(st_.baseline_uT, 3)
           + " noiseStd_uT=" + String(st_.noiseStd_uT, 3)
           + " dynThresh_uT=" + String(st_.dynThresh_uT, 3)
           + " mag_uT=" + String(mag_uT, 3)
           + " delta_uT=" + String(delta_uT, 3);
  return s;
}

DetectEvent VehicleDetector::update(float mag_uT, uint32_t now_ms) {
  // ── Calibration phase ──────────────────────────────────────────
  if (!calDone_ || isnan(st_.baseline_uT)) {
    if ((now_ms - startMs_) < CAL_TIME_MS) {
      calCount_++;
      float delta = mag_uT - calMean_;
      calMean_   += delta / calCount_;
      calM2_     += delta * (mag_uT - calMean_);
      return DetectEvent::Calibrating;
    } else {
      st_.baseline_uT = calMean_;
      st_.noiseStd_uT = (calCount_ > 1) ? sqrtf(calM2_ / (calCount_ - 1)) : 0.5f;
      st_.dynThresh_uT = fmaxf(ABS_THRESHOLD_UT, K_SIGMA * st_.noiseStd_uT);
      calDone_ = true;
      return DetectEvent::CalDone;
    }
  }

  // ── Tracking & detection ───────────────────────────────────────
  float delta_uT = fabsf(mag_uT - st_.baseline_uT);

  // Adapt baseline slowly only when not in active event
  if (!st_.eventActive) {
    st_.baseline_uT = (1.0f - BASELINE_ALPHA) * st_.baseline_uT + BASELINE_ALPHA * mag_uT;
  }

  const float highThresh = st_.dynThresh_uT;
  const float lowThresh  = st_.dynThresh_uT * HYSTERESIS_FRACTION;

  if (!st_.eventActive) {
    if (delta_uT >= highThresh) {
      if (++highCount_ >= N_CONSEC_HIGH) {
        st_.eventActive = true;
        highCount_ = 0; lowCount_ = 0;
        return DetectEvent::VehicleDetected;
      }
    } else {
      highCount_ = 0;
    }
  } else {
    if (delta_uT <= lowThresh) {
      if (++lowCount_ >= N_CONSEC_LOW) {
        st_.eventActive = false;
        lowCount_ = 0; highCount_ = 0;
        return DetectEvent::EventCleared;
      }
    } else {
      lowCount_ = 0;
    }
  }

  return DetectEvent::None;
}
