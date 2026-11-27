#include "M5Unified.h"

void setup(void) {
  M5.begin();

  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextFont(&fonts::FreeMonoBoldOblique9pt7b);
  M5.Display.setRotation(1);

  M5.Display.drawString("BtnA to Sleep 5s", M5.Display.width() / 2, M5.Display.height() / 2);
}

void loop(void) {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    M5.Display.clear();
    M5.Power.timerSleep(5);//sec
  }
}
