#include <M5Unified.h>
#include <M5GFX.h>
#include <M5UnitENV.h>

static constexpr int SHT_SDA_PIN   = 3;
static constexpr int SHT_SCL_PIN   = 2;

M5Canvas canvas(&M5.Display);

float temp, humi;
SHT4X sht4;
static constexpr uint32_t UPDATE_INTERVAL_MS = 120000;

static void renderScreen(float t, float h)
{
    char temp_text[32];
    char humi_text[32];
    snprintf(temp_text, sizeof(temp_text), "Temp: %.1f C", t);
    snprintf(humi_text, sizeof(humi_text), "Humi: %.1f %%", h);

    const int w = M5.Display.width();
    const int hgt = M5.Display.height();

    canvas.fillSprite(WHITE);
    canvas.setFont(&fonts::FreeMonoBold24pt7b);

    // Title centered at the top.
    canvas.setTextColor(BLACK);
    canvas.setTextDatum(top_center);
    canvas.drawString("SHT40 Sensor", w / 2, 20);

    // Measurements centered, each in its own color.
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(RED);
    canvas.drawString(temp_text, w / 2, hgt / 2 - 40);

    canvas.setTextColor(BLUE);
    canvas.drawString(humi_text, w / 2, hgt / 2 + 40);

    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg          = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.setFont(&fonts::FreeMonoBold24pt7b);
    M5.Display.setEpdMode(epd_mode_t::epd_fast);

    canvas.createSprite(M5.Display.width(), M5.Display.height());

    if (!sht4.begin(&Wire, SHT40_I2C_ADDR_44, SHT_SDA_PIN, SHT_SCL_PIN, 400000U))
    {
        canvas.fillSprite(WHITE);
        canvas.setFont(&fonts::FreeMonoBold24pt7b);
        canvas.setTextColor(RED);
        canvas.setTextDatum(middle_center);
        canvas.drawString("SHT40 not found", M5.Display.width() / 2, M5.Display.height() / 2);
        canvas.pushSprite(0, 0);
        while (1) delay(1);
    }

}

void loop()
{
    M5.update();

    static uint32_t last_update_ms = millis() - UPDATE_INTERVAL_MS;
    const uint32_t now_ms = millis();

    if (now_ms - last_update_ms >= UPDATE_INTERVAL_MS)
    {
        last_update_ms = now_ms;
        if (sht4.update())
        {
            temp = sht4.cTemp;
            humi = sht4.humidity;
            renderScreen(temp, humi);
        }
    }

    delay(100);
}
