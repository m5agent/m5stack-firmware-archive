#include <M5StickCPlus.h>

void setup() {
  M5.begin();
}

void loop() {
  M5.update();
  M5.Lcd.setCursor(0, 0);
  if (M5.BtnA.releasedFor(2000)) {
    M5.Lcd.println("Button A was released for more than 2 seconds.");
    delay(1000);
  }else{
    M5.Lcd.println("Button A is pressed                           ");
  }
}
