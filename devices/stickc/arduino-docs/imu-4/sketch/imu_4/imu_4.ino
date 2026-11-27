#include <M5StickC.h>

float pitch, roll, yaw;

void setup() {
  M5.begin();
  M5.IMU.Init();
}
void loop() {
  M5.IMU.getAhrsData(&pitch, &roll, &yaw);
  M5.Lcd.setCursor(0, 45);
  M5.Lcd.printf("X:%5.2f/nY:%5.2f/nZ:%5.2f ", pitch, roll, yaw);
  delay(500);
}
