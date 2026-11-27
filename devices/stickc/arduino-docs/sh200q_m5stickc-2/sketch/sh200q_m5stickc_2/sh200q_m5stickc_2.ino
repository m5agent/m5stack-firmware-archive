#include <M5StickC.h>

int16_t gyroX, gyroY, gyroZ;

void setup() {
  M5.begin();
  M5.IMU.Init();
}
void loop() {
  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf("X:%7.2f/nY:%7.2f/nZ:%7.2f mg",
                ((float)gyroX) * M5.IMU.gRes,
                ((float)gyroY) * M5.IMU.gRes,
                ((float)gyroZ) * M5.IMU.gRes);
  delay(500);
}
