#include "forwarder.h"
#include "config.h"

namespace Forward {

static uint32_t lastPingMs = 0;

static void printLinkConfig() {
  Serial.println();
  Serial.println(F("--- WAVESHARE -> MESHTASTIC UART BRIDGE ---"));
  Serial.printf("USB debug baud: %lu\n", 115200UL);
  Serial.printf("Link baud:      %lu\n", (unsigned long)LINK_BAUD);
  Serial.printf("Waveshare RX:   GPIO%d  <- Heltec TX GPIO43\n", LINK_RX_PIN);
  Serial.printf("Waveshare TX:   GPIO%d  -> Heltec RX GPIO44\n", LINK_TX_PIN);
  Serial.println(F("Meshtastic Serial Module should be: enabled, TEXTMSG, RX=44, TX=43"));
  Serial.println(F("-------------------------------------------"));
  Serial.println();
}

void begin() {
  Serial.begin(115200);

  // Give USB serial a short moment on boards where it matters.
  const uint32_t start = millis();
  while (!Serial && millis() - start < 1500) {
    delay(10);
  }

#if ENABLE_LINK_UART
  // ESP32 Arduino signature:
  // Serial1.begin(baud, config, rxPin, txPin)
  Serial1.begin(LINK_BAUD, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);
#endif

  printLinkConfig();

#if ENABLE_LINK_UART
  sendMeshText("BOOT: WAVESHARE_UART_ONLINE");
#endif
}

void poll() {
#if ENABLE_LINK_UART && LINK_DEBUG_RX_ECHO
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
  Serial.print(F("[DEBUG] "));
  Serial.println(line);
}

void sendMeshText(const String& msg) {
#if ENABLE_LINK_UART
  if (msg.length() == 0) return;

  Serial1.print(msg);
  Serial1.flush();

  Serial.print(F("[LINK-OUT] "));
  Serial.println(msg);
#endif
}

void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState& st,
               uint32_t t_ms) {
  String msg;

  switch (ev) {
    case DetectEvent::VehicleDetected:
      msg = "ALERT: VEHICLE";
      break;

    case DetectEvent::EventCleared:
      msg = "STATUS: CLEAR";
      break;

    case DetectEvent::CalDone:
      msg = "STATUS: READY";
      break;

    default:
      return;
  }

  Serial.printf("[EVENT] t=%lu mag=%.3f delta=%.3f baseline=%.3f noise=%.3f thresh=%.3f active=%d msg=\"%s\"\n",
                (unsigned long)t_ms,
                mag_uT,
                delta_uT,
                st.baseline_uT,
                st.noiseStd_uT,
                st.dynThresh_uT,
                st.eventActive ? 1 : 0,
                msg.c_str());

  sendMeshText(msg);
}

void sendTestPingIfDue() {
#if ENABLE_LINK_UART
  const uint32_t now = millis();

  if (lastPingMs == 0 || now - lastPingMs >= TEST_PING_PERIOD_MS) {
    lastPingMs = now;

    String msg = "HB_" + String(now / 1000);
    sendMeshText(msg);
  }
#endif
}

} // namespace Forward