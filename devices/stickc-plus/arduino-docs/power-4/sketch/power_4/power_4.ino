#include <M5StickCPlus.h>

uint8_t i = 0;

void setup() {
    M5.begin();  // By default, "M5.begin()" will initialize AXP192 chip
    M5.Lcd.printf("Hello, M5Stack!!");
}
void loop() {
    M5.Axp.ScreenBreath(i++);
    if (i > 100) i = 0;
    delay(10);
}
