#include "forwarder.h"
#include "config.h"

namespace Forward {

static uint32_t lastPingMs = 0;

void begin() {
  Serial.begin(115200);

  uint32_t t0 = millis();
  while (!Serial && (millis() - t0 < 1500)) {
    delay(10);
  }

#if ENABLE_LINK_UART
  Serial1.begin(LINK_BAUD, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);
#endif

#if ENABLE_USB_DEBUG && ENABLE_STATUS_LOGS
  Serial.println();
  Serial.println(F("Vehicle Detector - UART TEXTMSG mode for RAK Meshtastic"));
  Serial.printf("USB baud: 115200\n");

#if ENABLE_LINK_UART
  Serial.printf("UART baud: %lu\n", (unsigned long)LINK_BAUD);
  Serial.printf("Waveshare TX GPIO%d -> RAK RXD1 / pin 15\n", LINK_TX_PIN);
  Serial.printf("Waveshare RX GPIO%d <- RAK TXD1 / pin 16 optional\n", LINK_RX_PIN);
  Serial.println(F("UART message format: ASCII + CRLF"));
#else
  Serial.println(F("UART disabled."));
#endif

  Serial.println();
#endif

  sendMeshText("BOOT_RAK");
}

void poll() {
#if ENABLE_LINK_UART && LINK_DEBUG_RX_ECHO && ENABLE_USB_DEBUG
  while (Serial1.available() > 0) {
    int c = Serial1.read();

    Serial.print(F("[LINK-IN] 0x"));
    if (c < 16) Serial.print('0');
    Serial.print(c, HEX);
    Serial.print(F(" '"));

    if (c >= 32 && c <= 126) {
      Serial.print((char)c);
    } else if (c == '\n') {
      Serial.print(F("\\n"));
    } else if (c == '\r') {
      Serial.print(F("\\r"));
    } else {
      Serial.print('.');
    }

    Serial.println('\'');
  }
#endif
}

void sendDebug(const String& line) {
#if ENABLE_USB_DEBUG
  Serial.println(line);
#else
  (void)line;
#endif
}

void sendMeshText(const String& msg) {
#if ENABLE_LINK_UART
  if (msg.length() == 0) return;

  Serial1.print(msg);
  Serial1.print("\r\n");
  Serial1.flush();
#endif

#if ENABLE_USB_DEBUG
  Serial.print(F("[MESH-OUT] "));
  Serial.println(msg);
#else
  (void)msg;
#endif
}

void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState& st,
               uint32_t t_ms) {
  String usbLine;
  String meshLine;

  switch (ev) {
    case DetectEvent::VehicleDetected:
      usbLine = String("veh:1,ALARM")
              + ",mag="       + String(mag_uT, 3)
              + ",delta="     + String(delta_uT, 3)
              + ",baseline="  + String(st.baseline_uT, 3)
              + ",thresh="    + String(st.dynThresh_uT, 3)
              + ",t_ms="      + String(t_ms);

      meshLine = "VEH_ALARM";
      break;

    case DetectEvent::EventCleared:
      usbLine = String("veh:0,CLEAR")
              + ",mag="       + String(mag_uT, 3)
              + ",delta="     + String(delta_uT, 3)
              + ",baseline="  + String(st.baseline_uT, 3)
              + ",thresh="    + String(st.dynThresh_uT, 3)
              + ",t_ms="      + String(t_ms);

      meshLine = "VEH_CLEAR";
      break;

    case DetectEvent::CalDone:
      usbLine = String("veh:cal")
              + ",baseline="  + String(st.baseline_uT, 3)
              + ",noise="     + String(st.noiseStd_uT, 3)
              + ",thresh="    + String(st.dynThresh_uT, 3);

      meshLine = "VEH_READY";
      break;

    default:
      return;
  }

#if ENABLE_USB_DEBUG
  Serial.println(usbLine);
#else
  (void)usbLine;
#endif

  sendMeshText(meshLine);
}

void sendTestPingIfDue() {
#if ENABLE_TEST_PING && ENABLE_LINK_UART
  uint32_t now = millis();

  if (lastPingMs == 0 || now - lastPingMs >= TEST_PING_PERIOD_MS) {
    lastPingMs = now;
    sendMeshText("HB_RAK");
  }
#endif
}

} // namespace Forward