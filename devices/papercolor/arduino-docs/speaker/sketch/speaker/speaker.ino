#include <M5Unified.h>

static void drawScreen()
{
    M5.Display.fillScreen(WHITE);
    M5.Display.setTextColor(RED);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(4);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Speaker Tone Test", M5.Display.width() / 2, M5.Display.height() / 2);
}

void setup()
{
    M5.begin();
    M5.Display.setRotation(1);
    M5.Display.setEpdMode(epd_mode_t::epd_fast);

    drawScreen();
}

void loop()
{
    M5.Speaker.tone(4000, 200);
    delay(2000);
    M5.Speaker.tone(8000, 200);
    delay(2000);
}
