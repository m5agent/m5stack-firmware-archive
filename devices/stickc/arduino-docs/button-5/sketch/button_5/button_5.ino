#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.update();
  if (M5.BtnA.pressedFor(2000)) {
    M5.Lcd.println("Button A was pressed for more than 2 seconds.");
    delay(1000);
  }
}
