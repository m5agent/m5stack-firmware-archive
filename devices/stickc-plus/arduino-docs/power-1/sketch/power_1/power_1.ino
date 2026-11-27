#include <M5StickCPlus.h>

void setup() {
    M5.begin();
    M5.Lcd.begin();
    M5.Lcd.setRotation(3);
}

void loop() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0, 1);
    M5.Lcd.printf("AXP Temp: %.1fC \r\n", M5.Axp.GetTempInAXP192());
    M5.Lcd.printf("Bat:\r\n  V: %.3fv  I: %.3fma\r\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());
    M5.Lcd.printf("USB:\r\n  V: %.3fv  I: %.3fma\r\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());
    M5.Lcd.printf("5V-In:\r\n  V: %.3fv  I: %.3fma\r\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
    M5.Lcd.printf("Bat power %.3fmw", M5.Axp.GetBatPower());
    M5.update();
    delay(100);
}
