#include <M5Unified.h>

static void showMessage(const char* msg)
{
    M5.Display.fillScreen(WHITE);
    M5.Display.setTextColor(BLACK);
    M5.Display.setTextFont(4);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(middle_center);
    M5.Display.drawString(msg, M5.Display.width() / 2, M5.Display.height() / 2);
}

void setup()
{
    auto cfg          = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);
    M5.Display.setEpdMode(epd_mode_t::epd_fast);

    showMessage("Ready: press A / B / C");
}

void loop()
{
    M5.update();

    if (M5.BtnA.wasPressed()) Serial.println("A was Pressed");
    if (M5.BtnB.wasPressed()) Serial.println("B was Pressed");
    if (M5.BtnC.wasPressed()) Serial.println("C was Pressed");

    delay(100);
}
