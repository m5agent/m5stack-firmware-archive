#include <M5Unified.h>
#include <Adafruit_NeoPixel.h>

static constexpr uint32_t REFRESH_INTERVAL_MS = 60000;

M5Canvas canvas(&M5.Display);
uint32_t last_refresh_ms = 0;
Adafruit_NeoPixel pixels(2, 21, NEO_GRB + NEO_KHZ800);
bool has_last_vbus_state = false;
bool last_has_vbus = false;

static void setPowerLed(bool has_vbus)
{
    const uint32_t color = has_vbus ? pixels.Color(0, 255, 0) : pixels.Color(0, 0, 255);
    pixels.setPixelColor(0, color);
    pixels.setPixelColor(1, color);
    pixels.show();
}

static void drawPowerScreen()
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    const int batteryLevel = M5.Power.getBatteryLevel();
    const int batteryVoltage = M5.Power.getBatteryVoltage();
    const int vbusVoltage = M5.Power.getVBUSVoltage();
    const bool hasVbus = (vbusVoltage > 1000);

    canvas.fillSprite(WHITE);
    canvas.setTextDatum(top_center);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.setTextColor(BLACK);
    canvas.drawString("Power Status", w / 2, 26);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(BLUE);
    canvas.drawString("Refresh: 1 min", w / 2, 90);

    canvas.setTextColor(hasVbus ? GREEN : RED);
    canvas.drawString(hasVbus ? "Input Power: ON" : "Input Power: OFF", w / 2, 160);

    char buf[64];
    canvas.setTextColor(TFT_DARKCYAN);
    snprintf(buf, sizeof(buf), "Battery: %d%%", batteryLevel);
    canvas.drawString(buf, w / 2, 230);

    canvas.setTextColor(MAGENTA);
    snprintf(buf, sizeof(buf), "Voltage: %d mV", batteryVoltage);
    canvas.drawString(buf, w / 2, 290);

    canvas.setTextColor(BLACK);
    if (vbusVoltage >= 0) {
        snprintf(buf, sizeof(buf), "VBUS: %d mV", vbusVoltage);
    } else {
        snprintf(buf, sizeof(buf), "VBUS: N/A");
    }
    canvas.drawString(buf, w / 2, 350);

    canvas.pushSprite(0, 0);

    setPowerLed(hasVbus);

    Serial.printf("Power info refreshed: input=%d, level=%d%%, bat=%dmV, vbus=%dmV\n", hasVbus ? 1 : 0, batteryLevel,
                  batteryVoltage, vbusVoltage);
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);

    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    canvas.createSprite(M5.Display.width(), M5.Display.height());
    pixels.begin();
    pixels.setBrightness(80);
    pixels.clear();
    pixels.show();

    drawPowerScreen();
    last_refresh_ms = millis();
    last_has_vbus = (M5.Power.getVBUSVoltage() > 1000);
    has_last_vbus_state = true;
}

void loop()
{
    M5.update();

    const bool has_vbus_now = (M5.Power.getVBUSVoltage() > 1000);
    if (!has_last_vbus_state || has_vbus_now != last_has_vbus) {
        last_has_vbus = has_vbus_now;
        has_last_vbus_state = true;
        last_refresh_ms = millis();
        drawPowerScreen();
        delay(20);
        return;
    }

    const uint32_t now = millis();
    if (now - last_refresh_ms >= REFRESH_INTERVAL_MS) {
        last_refresh_ms = now;
        drawPowerScreen();
    }

    delay(20);
}
