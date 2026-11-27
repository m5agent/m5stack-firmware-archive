#include <M5Unified.h>

void setup(void)
{
    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setTextColor(RED);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::Orbitron_Light_24);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Touch Button Test", M5.Display.width() / 2, 15);
}

void loop(void)
{
    M5.update();

    if (M5.BtnPWR.wasClicked()) {
        M5.Display.fillRect(0, 40, M5.Display.width(), 70, BLACK);
        M5.Display.drawString("Btn PWR", M5.Display.width() / 2, M5.Display.height() / 2 - 30);
    }
}
