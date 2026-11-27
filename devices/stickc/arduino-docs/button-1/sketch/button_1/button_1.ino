#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Button A Status: %d ",M5.BtnA.read());
}
