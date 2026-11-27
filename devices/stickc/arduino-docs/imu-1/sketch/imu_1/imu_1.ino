#include <M5StickC.h>

void setup() {
  M5.begin();
  int x = M5.IMU.Init(); //return 0 is ok, return -1 is unknown. If the return value is 0, initialization is successful
  Serial.println(x);
}
void loop() {
}
