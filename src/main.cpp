// main.cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90393.h>
#include "config.h"

// =====================================================
// 1) SENSOR WRAPPER (MLX90393)
// =====================================================

Adafruit_MLX90393 mag;
bool sensorConfigured = false;

bool sensorBegin() {
  // Start I2C on chosen pins
  Wire.begin(MAG_I2C_SDA_PIN, MAG_I2C_SCL_PIN);

  // Use default address and pass Wire explicitly
  if (!mag.begin_I2C(MLX90393_DEFAULT_ADDR, &Wire)) {
    return false;
  }

  // Configure roughly as before (can be tuned later)
  mag.setGain(MLX90393_GAIN_1X);
  mag.setResolution(MLX90393_X, MLX90393_RES_17);
  mag.setResolution(MLX90393_Y, MLX90393_RES_17);
  mag.setResolution(MLX90393_Z, MLX90393_RES_16);
  mag.setOversampling(MLX90393_OSR_3);
  mag.setFilter(MLX90393_FILTER_5);

  sensorConfigured = true;
  return true;
}

// Read XYZ components (in microtesla)
bool readXYZ(float &x, float &y, float &z) {
  if (!sensorConfigured) return false;
  return mag.readData(&x, &y, &z);
}

// Read magnitude |B| in microtesla
bool readMagnitude(float &mag_uT) {
  float x, y, z;
  if (!readXYZ(x, y, z)) return false;
  mag_uT = sqrtf(x * x + y * y + z * z);
  return true;
}

// =====================================================
// 2) DETECTOR
// =====================================================

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
  VehicleDetector()
  : startMs_(millis())
  {}

  DetectEvent update(float mag_uT, uint32_t now_ms) {
    // --- 1) Calibration phase ---
    if (!calDone_ || isnan(state_.baseline_uT)) {
      if ((now_ms - startMs_) < CAL_TIME_MS) {
        calCount_++;
        float delta = mag_uT - calMean_;
        calMean_   += delta / calCount_;
        calM2_     += delta * (mag_uT - calMean_);
        return DetectEvent::Calibrating;
      } else {
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
          highCount_ = 0;
          lowCount_  = 0;
          return DetectEvent::VehicleDetected;  // ALARM
        }
      } else {
        highCount_ = 0;
      }
    } else {
      // in an active event: wait for enough “low” samples to clear
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

  const DetectorState &state() const { return state_; }

  // Simple debug line for USB Serial only
  String bench(float mag_uT, float delta_uT) const {
    String s = "[BENCH]"
             + String(" baseline_uT=") + String(state_.baseline_uT, 3)
             + String(" noiseStd_uT=") + String(state_.noiseStd_uT, 3)
             + String(" dynThresh_uT=") + String(state_.dynThresh_uT, 3)
             + String(" mag_uT=") + String(mag_uT, 3)
             + String(" delta_uT=") + String(delta_uT, 3);
    return s;
  }

private:
  // Welford stats during calibration
  uint32_t calCount_ = 0;
  float    calMean_  = 0.0f;
  float    calM2_    = 0.0f;

  uint32_t startMs_  = 0;
  bool     calDone_  = false;

  uint8_t  highCount_ = 0;
  uint8_t  lowCount_  = 0;

  DetectorState state_;
};

// =====================================================
// 3) FORWARDER (USB debug + RS485 text messages)
// =====================================================

namespace Forward {

void begin() {
  Serial.begin(115200);
  // Wait for USB Serial when tethered (can be removed later)
  while (!Serial) { delay(10); }

#if ENABLE_RS485_UART
  Serial1.begin(RS485_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
#endif

  Serial.println(F("\nVehicle Detector + RS485 forwarder"));
}

// Debug only – **does not** go onto RS485 / mesh
void sendDebug(const String &line) {
  Serial.println(line);
}

// Event -> human + machine friendly single line
void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState &st,
               uint32_t t_ms) {
  String line;

  switch (ev) {
    case DetectEvent::VehicleDetected:
      line = String("veh:1,ALARM")
           + ",mag="       + String(mag_uT, 3)
           + ",delta="     + String(delta_uT, 3)
           + ",baseline="  + String(st.baseline_uT, 3)
           + ",thresh="    + String(st.dynThresh_uT, 3)
           + ",t_ms="      + String(t_ms);
      break;

    case DetectEvent::EventCleared:
      line = String("veh:0,NO_ALARM")
           + ",mag="       + String(mag_uT, 3)
           + ",delta="     + String(delta_uT, 3)
           + ",baseline="  + String(st.baseline_uT, 3)
           + ",thresh="    + String(st.dynThresh_uT, 3)
           + ",t_ms="      + String(t_ms);
      break;

    case DetectEvent::CalDone:
      line = String("veh:cal")
           + ",baseline="  + String(st.baseline_uT, 3)
           + ",noise="     + String(st.noiseStd_uT, 3)
           + ",thresh="    + String(st.dynThresh_uT, 3);
      break;

    default:
      // We don't send “Calibrating” or “None” over RS485 for now.
      return;
  }

  // Always echo to USB:
  Serial.println(line);

  // Also send over RS485 to Meshtastic node:
#if ENABLE_RS485_UART
  Serial1.println(line);
#endif
}

} // namespace Forward

// =====================================================
// 4) GLOBALS & ARDUINO SETUP/LOOP
// =====================================================

VehicleDetector detector;
static uint32_t lastBenchMs = 0;

void setup() {
  Forward::begin();

  Serial.println(F("Init magnetometer..."));
  if (!sensorBegin()) {
    Serial.println(F("ERROR: MLX90393 not found (check wiring, address, power)."));
    while (true) {
      delay(500);
    }
  }
  Serial.println(F("Sensor OK. Calibrating..."));
}

void loop() {
  float mag;
  if (!readMagnitude(mag)) {
    Serial.println(F("Sensor read fail"));
    delay(SAMPLE_PERIOD_MS);
    return;
  }

  uint32_t now = millis();
  DetectEvent ev = detector.update(mag, now);
  const auto &st = detector.state();

  float delta_uT = (isnan(st.baseline_uT))
    ? NAN
    : fabsf(mag - st.baseline_uT);

  // 1) Send event lines (ALARM / NO_ALARM / CAL DONE) over RS485
  if (ev == DetectEvent::VehicleDetected ||
      ev == DetectEvent::EventCleared   ||
      ev == DetectEvent::CalDone) {
    Forward::sendEvent(ev, mag, delta_uT, st, now);
  }

  // 2) Periodic debug line over USB only
  if (!isnan(delta_uT) && (now - lastBenchMs > DEBUG_BENCH_PERIOD_MS)) {
    Forward::sendDebug(detector.bench(mag, delta_uT));
    lastBenchMs = now;
  }

  delay(SAMPLE_PERIOD_MS);
}
