#include <M5StickCPlus.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.update();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("The last change at %d ms /n",M5.BtnA.lastChange());
}
