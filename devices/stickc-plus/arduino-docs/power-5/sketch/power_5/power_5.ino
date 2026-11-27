#include <M5StickCPlus.h>

uint8_t i = 0;

void setup() {
    M5.begin();
    M5.Lcd.fillScreen(BLUE);
}
void loop() {
    M5.Axp.ScreenSwitch(true);
    delay(1000);
    M5.Axp.ScreenSwitch(false);
    delay(1000);
}
