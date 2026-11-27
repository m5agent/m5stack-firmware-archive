#include <M5Unified.h>

void setup(void) {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextFont(&fonts::FreeSerifBold9pt7b);
  M5.Display.setTextSize(1);

  M5.Display.drawString("Press BtnA", M5.Display.width() / 2, M5.Display.height() / 2 - 20);
  M5.Display.drawString("to Sleep 5s", M5.Display.width() / 2, M5.Display.height() / 2 + 20);
}

void loop(void) {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    M5.Display.clear();
    M5.Display.drawString("Sleep 5s", M5.Display.width() / 2, M5.Display.height() / 2 + 20);
    M5.Power.timerSleep(5);//sec
  }
}
