#include <M5Unified.h>
#include <M5PM1.h>
#include <Wire.h>

static constexpr uint32_t POWERON_SECONDS = 10;
static constexpr uint32_t POWEROFF_SECONDS = 10;

M5PM1 pm1;
M5Canvas canvas(&M5.Display);

static bool pm1_ready = false;

static void drawScreen(const char* status)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(WHITE);
    canvas.setTextDatum(top_left);
    canvas.setTextColor(BLACK);

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.drawString("M5PM1 TIMER", 16, 24);

    canvas.setFont(&fonts::FreeSansBold9pt7b);
    canvas.setTextColor(BLUE);
    canvas.drawString("BtnA: power on after 10s", 16, 86);

    canvas.setTextColor(pm1_ready ? GREEN : RED);
    canvas.drawString(status, 16, h - 40);

    canvas.pushSprite(0, 0);
}

void setup(void)
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);

    Serial.begin(115200);
    M5.Display.setEpdMode(epd_mode_t::epd_quality);

    canvas.createSprite(M5.Display.width(), M5.Display.height());

    m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);
    pm1_ready       = (err == M5PM1_OK);

    if (pm1_ready) {
        pm1.setLdoEnable(true);
    }

    if (pm1_ready) {
        Serial.println("PM1 init ok");
        drawScreen("PM1 ready");
    } else {
        Serial.printf("PM1 init failed: %d\n", (int)err);
        drawScreen("PM1 init failed");
    }
}

void loop(void)
{
    M5.update();

    if (!pm1_ready) {
        delay(20);
        return;
    }

    if (M5.BtnA.wasPressed()) {
        Serial.printf("BtnA: timer power on in %lu s\n", POWERON_SECONDS);
        m5pm1_err_t err = pm1.timerSet(POWERON_SECONDS, M5PM1_TIM_ACTION_POWERON);
        if (err != M5PM1_OK) {
            Serial.printf("timerSet POWERON failed: %d\n", (int)err);
            drawScreen("BtnA failed");
        }
        delay(1000);
        pm1.shutdown();
    }
    delay(20);
}
