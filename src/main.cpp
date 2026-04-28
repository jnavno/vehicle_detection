#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90393.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#include "config.h"
#include "vehicle_detector.h"
#include "forwarder.h"

// =====================================================
// 1) HARDWARE: NeoPixel + MLX90393
// =====================================================

Adafruit_NeoPixel pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_MLX90393 mag;
static bool sensorConfigured = false;

static bool sensorBegin() {
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

static bool readXYZ(float& x, float& y, float& z) {
  if (!sensorConfigured) return false;
  return mag.readData(&x, &y, &z);
}

static bool readMagnitude(float& mag_uT) {
  float x, y, z;
  if (!readXYZ(x, y, z)) return false;

  mag_uT = sqrtf(x * x + y * y + z * z);
  return true;
}

// =====================================================
// 2) OPTIONAL DETECTION GPIO OUTPUT - disabled in config
// =====================================================

static bool detectOutState = false;

static void setDetectOutput(bool active, const char* reason) {
#if ENABLE_DETECT_GPIO_OUT
  detectOutState = active;

#if DETECT_OUT_ACTIVE_HIGH
  digitalWrite(DETECT_OUT_PIN, active ? HIGH : LOW);
#else
  digitalWrite(DETECT_OUT_PIN, active ? LOW : HIGH);
#endif

#if ENABLE_STATUS_LOGS
  Serial.printf("[DETECT-OUT] %s reason=%s pin=%d level=%s\n",
                active ? "ACTIVE" : "CLEAR",
                reason,
                DETECT_OUT_PIN,
#if DETECT_OUT_ACTIVE_HIGH
                active ? "HIGH" : "LOW"
#else
                active ? "LOW" : "HIGH"
#endif
  );
#endif
#else
  (void)active;
  (void)reason;
#endif
}

static void initDetectOutput() {
#if ENABLE_DETECT_GPIO_OUT
  pinMode(DETECT_OUT_PIN, OUTPUT);
  setDetectOutput(false, "boot");
#endif
}

// =====================================================
// 3) APP STATE
// =====================================================

VehicleDetector detector;
static uint32_t lastBenchMs = 0;

static void heartbeat() {
  static uint32_t last = 0;
  static bool on = false;

  if (millis() - last > 500) {
    last = millis();
    on = !on;

    if (detector.state().eventActive) {
      // Alarm active: green blink
      pixel.setPixelColor(0, on ? pixel.Color(0, 40, 0) : pixel.Color(0, 0, 0));
    } else {
      // Normal heartbeat: blue blink
      pixel.setPixelColor(0, on ? pixel.Color(0, 0, 40) : pixel.Color(0, 0, 0));
    }

    pixel.show();
  }
}

static void sensorFailBlink() {
  pixel.setPixelColor(0, pixel.Color(40, 0, 0));
  pixel.show();
  delay(150);

  pixel.setPixelColor(0, pixel.Color(0, 0, 0));
  pixel.show();
  delay(150);
}

void setup() {
  pixel.begin();
  pixel.setBrightness(30);
  pixel.clear();
  pixel.show();

  initDetectOutput();

  Forward::begin();

  Serial.println(F("BOOT: sketch started"));
  Serial.println(F("BOOT: UART TEXTMSG mode to RAK"));
  Serial.println(F("BOOT: entering sensorBegin()..."));

  if (!sensorBegin()) {
    Serial.println(F("ERROR: MLX90393 not found"));
    Forward::sendMeshText("MAG_INIT_FAIL");

    setDetectOutput(false, "sensor_init_fail");

    while (true) {
      Forward::poll();
      sensorFailBlink();
    }
  }

  Serial.println(F("Sensor OK. Calibrating..."));
  Forward::sendMeshText("CAL");
}

void loop() {
  heartbeat();
  Forward::poll();
  Forward::sendTestPingIfDue();

  float mag_uT = NAN;

  if (!readMagnitude(mag_uT)) {
    static uint32_t lastReadFail = 0;
    uint32_t nowFail = millis();

    if (nowFail - lastReadFail > 30000) {
      lastReadFail = nowFail;
      Forward::sendDebug(F("Sensor read fail"));
      Forward::sendMeshText("MAG_READ_FAIL");
      setDetectOutput(false, "sensor_read_fail");
    }

    delay(SAMPLE_PERIOD_MS);
    return;
  }

  uint32_t now = millis();
  DetectEvent ev = detector.update(mag_uT, now);
  const auto& st = detector.state();

  float delta_uT = isnan(st.baseline_uT) ? NAN : fabsf(mag_uT - st.baseline_uT);

#if TUNING_MODE_CSV
  static uint32_t lastCsv = 0;

  if (now - lastCsv >= CSV_PERIOD_MS && !isnan(delta_uT)) {
    lastCsv = now;

    Serial.print(now);
    Serial.print(',');
    Serial.print(mag_uT, 3);
    Serial.print(',');
    Serial.print(delta_uT, 3);
    Serial.print(',');
    Serial.print(st.baseline_uT, 3);
    Serial.print(',');
    Serial.print(st.dynThresh_uT, 3);
    Serial.print(',');
    Serial.println(st.eventActive ? 1 : 0);
  }
#endif

  if (ev == DetectEvent::VehicleDetected) {
    setDetectOutput(true, "vehicle_detected");
  } else if (ev == DetectEvent::EventCleared) {
    setDetectOutput(false, "event_cleared");
  } else if (ev == DetectEvent::CalDone) {
    setDetectOutput(false, "cal_done");
  }

  if (ev == DetectEvent::VehicleDetected ||
      ev == DetectEvent::EventCleared ||
      ev == DetectEvent::CalDone) {
    Forward::sendEvent(ev, mag_uT, delta_uT, st, now);
  }

  if (!isnan(delta_uT) && (now - lastBenchMs > DEBUG_BENCH_PERIOD_MS)) {
    Forward::sendDebug(detector.bench(mag_uT, delta_uT));
    lastBenchMs = now;
  }

  delay(SAMPLE_PERIOD_MS);
}