#include <M5Unified.h>

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);

    M5.Led.setAutoDisplay(false);
    M5.Led.setBrightness(80);
}

void loop()
{
    M5.update();

    M5.Led.setAllColor(255, 0, 0);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(0, 255, 0);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(0, 0, 255);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(255, 255, 255);
    M5.Led.display();
    delay(1000);
}
