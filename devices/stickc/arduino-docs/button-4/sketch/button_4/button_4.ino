#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.println("Please pressed Button A.");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {	
    M5.Lcd.println("Button is pressed.");
  } 
  delay(20);
}
