#pragma once
#include <Arduino.h>
#include "vehicle_detector.h"   // DetectEvent, DetectorState

namespace Forward {

void begin();

// Debug only – USB serial
void sendDebug(const String& line);

// Event -> full USB detail + short mesh line to Serial1 (RS-485 transceiver)
void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState& st,
               uint32_t t_ms);

} // namespace Forward
