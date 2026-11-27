#include <M5StickC.h>

void setup() {
  uint8_t c;
  M5.begin();
  M5.IMU.Init();
  M5.IMU.I2C_Read_NBytes(SH200I_ADDRESS, SH200I_WHOAMI, 1, &c);
  M5.Lcd.print("SH200Q I AM "); M5.Lcd.println(c, HEX);
}
void loop() {}
