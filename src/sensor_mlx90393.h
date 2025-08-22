#pragma once
#include <Adafruit_MLX90393.h>

class MLX90393Sensor {
public:
  bool begin();
  bool readMagnitude(float& mag_uT);
  bool readXYZ(float& x, float& y, float& z);

private:
  Adafruit_MLX90393 sensor_;
  bool configured_ = false;
};
