#include <M5Unified.h>
#include <M5PM1.h>
#include <Adafruit_NeoPixel.h>

static constexpr uint8_t LED_PIN   = 21;
static constexpr uint8_t LED_COUNT = 2;

M5PM1 pm1;
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
static bool pm1_ready = false;

static void setSingleLed(uint8_t index, uint32_t color)
{
    if (index >= LED_COUNT) return;
    pixels.setPixelColor(index, color);
    pixels.show();
}

static void setAllLeds(uint32_t color)
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        pixels.setPixelColor(i, color);
    }
    pixels.show();
}

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
    M5.Display.setEpdMode(epd_mode_t::epd_fast);

    m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);
    pm1_ready       = (err == M5PM1_OK);

    if (pm1_ready)
    {
        pm1.setLdoEnable(true);
    }

    pixels.begin();
    pixels.setBrightness(80);
    setAllLeds(pixels.Color(255, 0, 0));

    showMessage("RGB LED TEST");
}

void loop()
{
    setAllLeds(0);
    setSingleLed(0, pixels.Color(255, 255, 255));
    delay(500);
    setAllLeds(0);
    setSingleLed(1, pixels.Color(255, 255, 255));
    delay(500);

    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setPixelColor(1, pixels.Color(0, 0, 255));
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.setPixelColor(1, pixels.Color(255, 0, 0));
    pixels.show();
    delay(500);

    setAllLeds(pixels.Color(255, 0, 0));
    delay(500);
    setAllLeds(pixels.Color(0, 255, 0));
    delay(500);
    setAllLeds(pixels.Color(0, 0, 255));
    delay(500);
}
