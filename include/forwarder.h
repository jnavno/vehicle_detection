#pragma once
#include <Arduino.h>
#include "vehicle_detector.h"

namespace Forward {

void begin();
void poll();

void sendDebug(const String& line);

void sendMeshText(const String& msg);

void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               const DetectorState& st,
               uint32_t t_ms);

void sendTestPingIfDue();

} // namespace Forward