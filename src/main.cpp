#include <Arduino.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

// Optionally track previous heading
int lastAzimuth = -1;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize compass
  compass.init();

  // Optional: apply calibration data (replace with your own if available)
  // compass.setCalibration(Xmin, Xmax, Ymin, Ymax, Zmin, Zmax);

  // Optional: set smoothing with advanced filtering
  compass.setSmoothing(10, true);

  // Optional: set I2C address (if using non-default)
  // compass.setADDR(0x0D);

  Serial.println("QMC5883L Compass initialized.");
}

void loop() {
  compass.read();

  int x = compass.getX();
  int y = compass.getY();
  int z = compass.getZ();
  int azimuth = compass.getAzimuth();
  byte bearing = compass.getBearing(azimuth);

  char direction[3];
  compass.getDirection(direction, azimuth);

  Serial.printf("📡 X: %d Y: %d Z: %d | Azimuth: %d° | Bearing: %d | Dir: %c%c%c\n",
                x, y, z, azimuth, bearing,
                direction[0], direction[1], direction[2]);

  // You can now add logic like:
  // if (abs(azimuth - lastAzimuth) > threshold) triggerMeshtastic();

  lastAzimuth = azimuth;

  delay(1000);
}
