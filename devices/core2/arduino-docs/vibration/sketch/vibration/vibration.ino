#include <M5Unified.h>

void setup(void) {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextFont(&fonts::Orbitron_Light_24);
  M5.Display.setTextSize(1);
  M5.Display.drawString("Vibration Test", M5.Display.width() / 2, M5.Display.height() / 2);
}

void loop(void) {
  M5.Power.setVibration(10);
  delay(500);
  M5.Power.setVibration(40);
  delay(500);
  M5.Power.setVibration(70);
  delay(500);
  M5.Power.setVibration(100);
  delay(500);
  M5.Power.setVibration(130);
  delay(500);
  M5.Power.setVibration(160);
  delay(500);
  M5.Power.setVibration(190);
  delay(500);
  M5.Power.setVibration(210);
  delay(500);
  M5.Power.setVibration(240);
  delay(500);
}
