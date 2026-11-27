#include <M5StickCPlus.h>

void setup() {
  M5.begin();
  M5.Lcd.setCursor(7, 20);
  M5.Lcd.println("scan done");
  M5.Lcd.setCursor(5, 60);
  M5.Lcd.printf("50 AP");
}
void loop(){}
