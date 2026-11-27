#include "M5Unified.h"

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.setTextColor(GREEN);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(&fonts::FreeSerifBoldItalic18pt7b);
    M5.Display.setTextSize(1);

    M5.Power.setBatteryCharge(true);
    M5.Power.setChargeCurrent(100);
    M5.Power.setChargeVoltage(4200);
}

void loop()
{
    M5.Display.clear();
    int vol = M5.Power.getBatteryVoltage();
    M5.Display.setCursor(10, 30);
    M5.Display.printf("Bat: %dmv", vol);
    int level = M5.Power.getBatteryLevel();
    M5.Display.setCursor(10, 65);
    M5.Display.printf("Level: %d%%", level);
    M5.Display.setCursor(10, 100);
    m5::Power_Class::is_charging_t status = M5.Power.isCharging();
    M5.Display.printf("Charging: %s", status == m5::Power_Class::is_charging_t::is_charging ? "YES" : "NO");
    delay(1000);
}
