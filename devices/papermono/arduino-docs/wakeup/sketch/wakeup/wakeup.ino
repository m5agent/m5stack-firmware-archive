#include <M5Unified.h>

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setEpdMode(epd_mode_t::epd_text);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setFont(&fonts::FreeSansBold18pt7b);
    M5.Display.clear(TFT_WHITE);
    M5.Display.drawString("PaperMono Wakeup Test", M5.Display.width() / 2, 120);

    M5.Display.setFont(&fonts::FreeSansBold12pt7b);
    M5.Display.drawString("BtnA: Light Sleep 5s", M5.Display.width() / 2, 260);
    M5.Display.drawString("BtnB: Depp Sleep 5s", M5.Display.width() / 2, 320);
    M5.Display.drawString("Press BtnA or BtnB", M5.Display.width() / 2, 400);
}

void loop()
{
    M5.update();

    if (M5.BtnA.wasPressed()) {
        M5.Display.clear(TFT_WHITE);
        M5.Display.setTextDatum(middle_center);
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.setFont(&fonts::FreeSansBold18pt7b);
        M5.Display.drawString("BtnA: Light Sleep", M5.Display.width() / 2, 180);
        M5.Display.setFont(&fonts::FreeSansBold12pt7b);
        M5.Display.drawString("Sleeping for 5 seconds", M5.Display.width() / 2, 280);
        delay(200);
        M5.Power.lightSleep(5000000, true);

        M5.Display.clear(TFT_WHITE);
        M5.Display.setTextDatum(middle_center);
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.setFont(&fonts::FreeSansBold18pt7b);
        M5.Display.drawString("BtnA: Wakeup complete", M5.Display.width() / 2, 180);
        M5.Display.setFont(&fonts::FreeSansBold12pt7b);
        M5.Display.drawString("BtnB: Deep Sleep 5s", M5.Display.width() / 2, 320);
    }

    if (M5.BtnB.wasPressed()) {
        M5.Display.clear(TFT_WHITE);
        M5.Display.setTextDatum(middle_center);
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.setFont(&fonts::FreeSansBold18pt7b);
        M5.Display.drawString("BtnB: Deep Sleep", M5.Display.width() / 2, 180);
        M5.Display.setFont(&fonts::FreeSansBold12pt7b);
        M5.Display.drawString("Wake up after 5 seconds", M5.Display.width() / 2, 280);
        delay(200);
        M5.Power.deepSleep(5000000, true);
    }

    delay(20);
} 
