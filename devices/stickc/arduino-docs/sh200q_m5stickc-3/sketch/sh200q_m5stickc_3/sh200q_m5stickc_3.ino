#include <M5StickC.h>

int16_t accX, accY, accZ;

void setup() {
  M5.begin();
  M5.IMU.Init();
}
void loop() {
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  M5.Lcd.setCursor(0, 45);
  M5.Lcd.printf("X:%5.2f/nY:%5.2f/nZ:%5.2f o/s",
                ((float)accX) * M5.IMU.aRes,
                ((float)accY) * M5.IMU.aRes,
                ((float)accZ) * M5.IMU.aRes);
  delay(500);
}
