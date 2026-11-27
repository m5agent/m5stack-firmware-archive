#include <M5Unified.h>

M5Canvas canvas(&M5.Display);
const char* lastAction = "Press BtnA or BtnB";

void drawStatus()
{
    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString("PaperMono Button Test", M5.Display.width() / 2, 160);
    canvas.drawString("LastAction:", M5.Display.width() / 2, 300);
    canvas.drawString(lastAction, M5.Display.width() / 2, 400);
    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setEpdMode(epd_mode_t::epd_text);
    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawStatus();
}

void loop()
{
    M5.update();
    if (M5.BtnA.wasPressed()) {
        lastAction = "BtnA pressed";
        drawStatus();
    }
    if (M5.BtnB.wasPressed()) {
        lastAction = "BtnB pressed";
        drawStatus();
    }
    delay(20);
}
