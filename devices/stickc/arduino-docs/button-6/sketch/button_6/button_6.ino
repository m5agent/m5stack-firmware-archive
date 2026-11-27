#include <M5StickC.h>

void setup() {
  M5.begin();
}

void loop() {
  M5.update();
  if (M5.BtnA.isReleased()) {
    M5.Lcd.println("Button is released.");
  }else{
    M5.Lcd.println("Button is Pressed .");
  }
  delay(20);
}
