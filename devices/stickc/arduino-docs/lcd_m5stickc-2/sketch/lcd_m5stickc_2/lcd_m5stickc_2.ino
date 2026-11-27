#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.setTextColor(RED, WHITE);
  M5.Lcd.println("Hello, M5Stack world!!");
}
void loop() {
}
