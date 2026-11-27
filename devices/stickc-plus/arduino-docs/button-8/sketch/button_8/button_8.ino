#include <M5StickCPlus.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.update();
  if(M5.BtnA.wasReleased()) {
    M5.Lcd.println("Button is Released.");
  }
  delay(20);
}
