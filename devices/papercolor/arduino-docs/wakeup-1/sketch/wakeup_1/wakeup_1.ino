#include <M5Unified.h>

M5Canvas canvas(&M5.Display);

static void drawEpdSleepScreen()
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(WHITE);
    canvas.setTextDatum(middle_center);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(RED);
    canvas.drawString("Press Top BtnA", w / 2, h / 2 - 20);

    canvas.setTextColor(GREEN);
    canvas.drawString("Sleep 5s, then wakeup", w / 2, h / 2 + 20);

    canvas.pushSprite(0, 0);
}

void setup(void)
{
    auto cfg          = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);

    M5.Display.setEpdMode(epd_mode_t::epd_quality);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawEpdSleepScreen();
}

void loop(void)
{
    M5.update();

    if (M5.BtnA.wasPressed()) {
        M5.Power.timerSleep(5);
    }

    delay(10);
}
