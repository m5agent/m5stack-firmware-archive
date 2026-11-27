#include <M5StickCPlus.h>

void setup() {
  M5.begin();
  M5.Lcd.fillTriangle(22, 22, 69, 98, 51, 22, RED);
}
void loop() {}
