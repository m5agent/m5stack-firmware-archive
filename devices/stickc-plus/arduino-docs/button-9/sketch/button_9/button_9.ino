#include <M5StickCPlus.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasReleasefor(3000)) {
      M5.Lcd.println("OK");
  }
}
