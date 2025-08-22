#pragma once
#include <Arduino.h>
#include "detector.h"

namespace Forward {

void begin();                               // set up optional Serial1
void sendLine(const String& line);          // prints to Serial (+ Serial1 if enabled)
void sendEvent(DetectEvent ev,
               float mag_uT,
               float delta_uT,
               float baseline_uT,
               float thresh_uT,
               uint32_t t_ms);

} // namespace Forward
