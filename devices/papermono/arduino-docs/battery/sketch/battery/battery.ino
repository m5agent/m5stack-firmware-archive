#include <M5Unified.h>

M5Canvas canvas(&M5.Display);

void drawBatteryStatus()
{
    const int w = M5.Display.width();
    const bool charging = M5.Power.isCharging() == m5::Power_Class::is_charging;
    const int level = M5.Power.getBatteryLevel();
    const int voltage = M5.Power.getBatteryVoltage();
    const int vbusVoltage = M5.Power.getVBUSVoltage();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(top_center);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString("PaperMono Battery Test", w / 2, 42);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString(charging ? "Charging: Yes" : "Charging: No", w / 2, 170);

    char text[40];
    snprintf(text, sizeof(text), "Battery: %d%%", level);
    canvas.drawString(text, w / 2, 270);
    snprintf(text, sizeof(text), "Voltage: %d mV", voltage);
    canvas.drawString(text, w / 2, 370);

    if (vbusVoltage >= 0) {
        snprintf(text, sizeof(text), "VBUS: %d mV", vbusVoltage);
    } else {
        snprintf(text, sizeof(text), "VBUS: N/A");
    }
    canvas.drawString(text, w / 2, 470);
    canvas.drawString("Refresh: 10 sec", w / 2, 590);
    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setEpdMode(epd_mode_t::epd_text);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawBatteryStatus();
}

void loop()
{
    M5.update();
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate >= 10000) {
        lastUpdate = millis();
        drawBatteryStatus();
    }
    delay(20);
}
