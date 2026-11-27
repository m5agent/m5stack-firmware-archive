#include <M5Unified.h>

M5Canvas canvas(&M5.Display);
static constexpr const char* const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static int last_drawn_minute = -1;

void drawScreen(const m5::rtc_datetime_t& dt)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    char dateText[32];
    char timeText[16];
    snprintf(dateText, sizeof(dateText), "%04d/%02d/%02d", dt.date.year, dt.date.month, dt.date.date);
    snprintf(timeText, sizeof(timeText), "%02d:%02d", dt.time.hours, dt.time.minutes);

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextColor(TFT_BLACK);
    canvas.setTextDatum(top_center);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString("PaperMono RTC Test", w / 2, 24);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.setTextSize(1);
    canvas.drawString(timeText, w / 2, h / 2 - 80);

    canvas.setTextSize(1);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString(dateText, w / 2, h / 2 + 48);
    canvas.drawString(wd[dt.date.weekDay], w / 2, h / 2 + 94);

    canvas.pushSprite(0, 0);
}

void drawErrorScreen(const char* message)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_BLACK);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString("PaperMono RTC Test", w / 2, h / 2 - 48);
    canvas.drawString(message, w / 2, h / 2 + 36);
    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setRotation(0);
    M5.Display.setEpdMode(epd_mode_t::epd_text);

    canvas.createSprite(M5.Display.width(), M5.Display.height());

    if (!M5.Rtc.isEnabled()) {
        drawErrorScreen("RTC not found");
        while (true) {
            delay(1000);
        }
    }

    M5.Rtc.setDateTime({{2026, 8, 18}, {12, 0, 0}});

    const auto dt = M5.Rtc.getDateTime();
    drawScreen(dt);
    last_drawn_minute = dt.time.minutes;
}

void loop()
{
    M5.update();
    const auto dt = M5.Rtc.getDateTime();
    if (dt.time.minutes != last_drawn_minute) {
        last_drawn_minute = dt.time.minutes;
        drawScreen(dt);
    }
    delay(100);
}
