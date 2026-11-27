#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include <M5PM1.h>

M5Canvas canvas(&M5.Display);
M5PM1 pm1;

static constexpr const char* const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static int last_drawn_minute = -1;
static bool pm1_ready = false;

static void drawScreen(const m5::rtc_datetime_t& dt)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    char date_buf[32];
    char hm_buf[16];

    snprintf(date_buf, sizeof(date_buf), "%04d/%02d/%02d", dt.date.year, dt.date.month, dt.date.date);
    snprintf(hm_buf, sizeof(hm_buf), "%02d:%02d", dt.time.hours, dt.time.minutes);

    canvas.fillSprite(WHITE);
    canvas.setTextDatum(top_center);
    canvas.setTextSize(1);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.setTextColor(BLACK);
    canvas.drawString("PAPER COLOR RTC", w / 2, 18);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.setTextSize(2);
    canvas.setTextColor(RED);
    canvas.drawString(hm_buf, w / 2, h / 2 - 36);

    canvas.setTextSize(1);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(BLUE);
    canvas.drawString(date_buf, w / 2, h / 2 + 42);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(GREEN);
    canvas.drawString(wd[dt.date.weekDay], w / 2, h / 2 + 86);

    canvas.pushSprite(0, 0);
}

static void drawErrorScreen(const char* message)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.setTextColor(RED);
    canvas.drawString(message, w / 2, h / 2);
    canvas.pushSprite(0, 0);
}

void setup(void)
{
    auto cfg          = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    M5.Display.setRotation(1);

    canvas.createSprite(M5.Display.width(), M5.Display.height());

    m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);
    pm1_ready       = (err == M5PM1_OK);

    if (pm1_ready) {
        pm1.setLdoEnable(true);
    }

    if (!M5.Rtc.isEnabled()) {
        Serial.println("RTC not found.");
        drawErrorScreen("RTC not found.");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println("RTC found.");

    M5.Rtc.setDateTime({{2026, 5, 8}, {12, 0, 0}});

    Serial.println("RTC demo time set to 2026/05/08 12:00:00");

    auto dt = M5.Rtc.getDateTime();
    drawScreen(dt);
    last_drawn_minute = dt.time.minutes;
}

void loop(void)
{
    auto dt = M5.Rtc.getDateTime();
    if (dt.time.minutes != last_drawn_minute) {
        last_drawn_minute = dt.time.minutes;
        Serial.printf("RTC: %04d/%02d/%02d (%s) %02d:%02d:%02d\r\n", dt.date.year, dt.date.month, dt.date.date,
                      wd[dt.date.weekDay], dt.time.hours, dt.time.minutes, dt.time.seconds);

        drawScreen(dt);
    }

    delay(100);
}
