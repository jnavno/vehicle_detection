#include "forwarder.h"
#include "config.h"

namespace Forward {

void begin() {
  Serial.begin(115200);

  // Don’t hang forever if running untethered
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0 < 1500)) { delay(10); }

#if ENABLE_LINK_UART
  Serial1.begin(LINK_BAUD, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);
#endif

  Serial.println(F("\nVehicle Detector + RS485 forwarder"));
}

void sendDebug(const String& line) {
  Serial.println(line);
}

void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState& st,
               uint32_t t_ms) {

  // 1) Full detail line for USB logs
  String usbLine;

  // 2) Short line for the Meshtastic serial input (Serial Module TEXTMSG)
  String meshLine;

  switch (ev) {
    case DetectEvent::VehicleDetected:
      usbLine = String("veh:1,ALARM")
              + ",mag="       + String(mag_uT, 3)
              + ",delta="     + String(delta_uT, 3)
              + ",baseline="  + String(st.baseline_uT, 3)
              + ",thresh="    + String(st.dynThresh_uT, 3)
              + ",t_ms="      + String(t_ms);

      meshLine = String("VEH ALARM d=") + String(delta_uT, 1);
      break;

    case DetectEvent::EventCleared:
      usbLine = String("veh:0,NO_ALARM")
              + ",mag="       + String(mag_uT, 3)
              + ",delta="     + String(delta_uT, 3)
              + ",baseline="  + String(st.baseline_uT, 3)
              + ",thresh="    + String(st.dynThresh_uT, 3)
              + ",t_ms="      + String(t_ms);

      meshLine = "VEH CLEAR";
      break;

    case DetectEvent::CalDone:
      usbLine = String("veh:cal")
              + ",baseline="  + String(st.baseline_uT, 3)
              + ",noise="     + String(st.noiseStd_uT, 3)
              + ",thresh="    + String(st.dynThresh_uT, 3);

      meshLine = "VEH CAL OK";
      break;

    default:
      return;
  }

  // echoing full detail to USB
  Serial.println(usbLine);

  // Sending short line over UART->RS485 to Meshtastic node
#if ENABLE_LINK_UART
  if (meshLine.length() > 0) {
    Serial1.println(meshLine);
  }
#endif
}

void sendMeshText(const String& msg) {
#if ENABLE_LINK_UART
  if (msg.length() > 0) {
    Serial1.println(msg);
  }
#endif
  // also echo to USB so you can see exactly what was sent
  Serial.print(F("[MESH] "));
  Serial.println(msg);
}

void sendTestPingIfDue() {
#if ENABLE_TEST_PING
  static uint32_t lastPing = 0;
  uint32_t now = millis();
  if (now - lastPing >= TEST_PING_PERIOD_MS) {
    lastPing = now;
    sendMeshText("TG TEST OK");
  }
#endif
}



} // namespace Forward
