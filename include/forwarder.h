#pragma once
#include <Arduino.h>
#include "vehicle_detector.h"

namespace Forward {

void begin();
void poll();

void sendDebug(const String& line);

void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState& st,
               uint32_t t_ms);

void sendMeshText(const String& msg);
void sendTestPingIfDue();

} // namespace Forward