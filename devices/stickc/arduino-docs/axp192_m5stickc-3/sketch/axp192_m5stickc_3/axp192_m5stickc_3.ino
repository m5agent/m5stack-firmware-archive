#include <M5StickC.h>

void setup() {
    M5.begin();
    M5.lcd.fillScreen(GREEN);
    delay(3000);
    M5.Axp.PowerOff();
}

void loop() {
}
