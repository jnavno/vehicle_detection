#include "sensor_mlx90393.h"
#include "config.h"
#include <Wire.h>

bool MLX90393Sensor::begin() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Use default address; pass Wire explicitly
  if (!sensor_.begin_I2C(MLX90393_DEFAULT_ADDR, &Wire)) {
    return false;
  }

  // Configure close to your original snippet
  sensor_.setGain(MLX90393_GAIN_1X);
  sensor_.setResolution(MLX90393_X, MLX90393_RES_17);
  sensor_.setResolution(MLX90393_Y, MLX90393_RES_17);
  sensor_.setResolution(MLX90393_Z, MLX90393_RES_16);
  sensor_.setOversampling(MLX90393_OSR_3);
  sensor_.setFilter(MLX90393_FILTER_5);

  configured_ = true;
  return true;
}

bool MLX90393Sensor::readXYZ(float& x, float& y, float& z) {
  if (!configured_) return false;
  return sensor_.readData(&x, &y, &z);
}

bool MLX90393Sensor::readMagnitude(float& mag_uT) {
  float x, y, z;
  if (!readXYZ(x, y, z)) return false;
  mag_uT = sqrtf(x*x + y*y + z*z);
  return true;
}
