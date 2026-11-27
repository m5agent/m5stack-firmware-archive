#include <M5StickCPlus.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.update();
  M5.Lcd.setCursor(0, 0);
  if (M5.BtnA.isPressed()) {
    M5.Lcd.println("Button is Pressed.");
  }else{
    M5.Lcd.println("Button is Released.");
  }
  delay(20);
}
