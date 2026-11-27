#include <M5StickC.h>

void setup() {
  M5.begin();
  M5.Lcd.drawLine(0, 0, 12, 12, BLUE);
}
void loop() {}
