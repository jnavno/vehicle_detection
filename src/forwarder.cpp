#include "forwarder.h"
#include "config.h"

namespace Forward {

void begin() {
#if ENABLE_UART1
  Serial1.begin(UART1_BAUD, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
#endif
}

void sendLine(const String& line) {
  Serial.println(line);
#if ENABLE_UART1
  Serial1.println(line);
#endif
}

void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               float baseline_uT,
               float thresh_uT,
               uint32_t t_ms) {
  switch (ev) {
    case DetectEvent::VehicleDetected: {
      String line = String("VEHICLE_DETECTED")
          + ",mag_uT="      + String(mag_uT, 3)
          + ",delta_uT="    + String(delta_uT, 3)
          + ",baseline_uT=" + String(baseline_uT, 3)
          + ",thresh_uT="   + String(thresh_uT, 3)
          + ",t_ms="        + String(t_ms);
      sendLine(line);
      break;
    }
    case DetectEvent::EventCleared:
      sendLine("EVENT_CLEAR");
      break;
    case DetectEvent::CalDone: {
      String line = String("CAL_DONE")
          + ",baseline_uT=" + String(baseline_uT, 3)
          + ",noiseStd_uT=" + String(thresh_uT / K_SIGMA, 3) // back-calc approx
          + ",thresh_uT="   + String(thresh_uT, 3);
      sendLine(line);
      break;
    }
    default:
      break;
  }
}

} // namespace Forward
