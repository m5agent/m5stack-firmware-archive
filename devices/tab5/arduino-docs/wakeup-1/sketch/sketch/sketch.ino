#include <M5Unified.h>

void setup(void) {
    auto cfg = M5.config();
    M5.begin(cfg);
    if(M5.Rtc.isEnabled()){
        M5.Display.drawString("OK", M5.Display.width() / 2,
                              M5.Display.height() / 2 - 60);
        delay(2000);
    }

    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::FreeMonoBold18pt7b);
    M5.Display.setRotation(1);

    M5.Display.clear();

    M5.Display.drawString("Touch to sleep", M5.Display.width() / 2, M5.Display.height() / 2 - 20);
    M5.Display.drawString("After 5s wakeup", M5.Display.width() / 2, M5.Display.height() / 2 + 20);
}

void loop(void) {
    M5.update();

    if (M5.Touch.getDetail(0).wasClicked()) {
        M5.Power.deepSleep(5000000ULL, false);
    }
}
