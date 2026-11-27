#include <M5Unified.h>
#include <M5PM1.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

static constexpr uint32_t POWERON_SECONDS = 10;
static constexpr uint32_t POWEROFF_SECONDS = 10;
static constexpr uint8_t LED_PIN = 21;
static constexpr uint8_t LED_COUNT = 2;

M5PM1 pm1;
M5Canvas canvas(&M5.Display);
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

static bool pm1_ready = false;

static void setAllLeds(uint32_t color)
{
    for (uint8_t i = 0; i < LED_COUNT; ++i) {
        pixels.setPixelColor(i, color);
    }
    pixels.show();
}

static void drawScreen(const char* status)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(WHITE);
    canvas.setTextDatum(top_center);
    canvas.setTextColor(BLACK);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.drawString("M5PM1 TIMER", w / 2, 34);

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.setTextColor(BLUE);
    canvas.drawString("BtnA: Power ON in 10s", w / 2, 180);
    canvas.drawString("BtnB: Power OFF in 10s", w / 2, 248);

    canvas.setTextColor(pm1_ready ? GREEN : RED);
    canvas.drawString(status, w / 2, h - 110);

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

    pixels.begin();
    pixels.setBrightness(60);
    setAllLeds(0);

    if (pm1_ready) {
        Serial.println("PM1 init ok");
        drawScreen("PM1 ready");
        setAllLeds(pixels.Color(0, 255, 0));
    } else {
        Serial.printf("PM1 init failed: %d\n", (int)err);
        drawScreen("PM1 init failed");
        setAllLeds(pixels.Color(255, 0, 0));
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
        setAllLeds(pixels.Color(255, 255, 0));
        m5pm1_err_t err = pm1.timerSet(POWERON_SECONDS, M5PM1_TIM_ACTION_POWERON);
        if (err != M5PM1_OK) {
            Serial.printf("timerSet POWERON failed: %d\n", (int)err);
            drawScreen("BtnA failed");
            setAllLeds(pixels.Color(255, 0, 0));
        }
        delay(1000);
        pm1.shutdown();
    }

    if (M5.BtnB.wasPressed()) {
        Serial.printf("BtnB: timer poweroff in %lu s\n", POWEROFF_SECONDS);
        setAllLeds(pixels.Color(0, 0, 255));
        m5pm1_err_t err = pm1.timerSet(POWEROFF_SECONDS, M5PM1_TIM_ACTION_POWEROFF);
        if (err != M5PM1_OK) {
            Serial.printf("timerSet POWEROFF failed: %d\n", (int)err);
            drawScreen("BtnB failed");
            setAllLeds(pixels.Color(255, 0, 0));
        }
    }

    delay(20);
}
