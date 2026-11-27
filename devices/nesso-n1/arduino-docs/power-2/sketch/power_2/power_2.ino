#include "M5Unified.h"

void setup() {
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

bool power_status = true;

void loop() {
  M5.Display.clear();
  M5.Display.setCursor(10, 30);
  M5.Display.println("Grove Power:");
  M5.Power.setExtOutput(power_status);
  M5.Display.setCursor(10, 65);
  M5.Display.printf("%s", power_status ? "ON" : "OFF");
  power_status = !power_status;
  delay(1000);
}
