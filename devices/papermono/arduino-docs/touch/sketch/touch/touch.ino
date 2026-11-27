#include <M5Unified.h>

constexpr uint_fast8_t MAX_TOUCH_POINTS = 2; 

M5Canvas canvas(&M5.Display);
m5::touch_point_t touchPoints[MAX_TOUCH_POINTS];
m5::touch_point_t lastPoints[MAX_TOUCH_POINTS];
uint_fast8_t lastCount = 0;
uint32_t lastRefresh = 0;

bool touchChanged(const m5::touch_point_t* points, uint_fast8_t count)
{
    if (count != lastCount) {
        return true;
    }

    for (uint_fast8_t i = 0; i < count; ++i) {
        if (points[i].x != lastPoints[i].x || points[i].y != lastPoints[i].y
            || points[i].id != lastPoints[i].id) {
            return true;
        }
    }
    return false;
}

void saveTouchPoints(const m5::touch_point_t* points, uint_fast8_t count)
{
    lastCount = count;
    for (uint_fast8_t i = 0; i < count; ++i) {
        lastPoints[i] = points[i];
    }
}

void drawTouchPoints(const m5::touch_point_t* points, uint_fast8_t count)
{
    const int w = M5.Display.width();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(top_center);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString("PaperMono Touch Test", w / 2, 30);

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    char text[40];
    snprintf(text, sizeof(text), "Touch points: %u / %u", static_cast<unsigned>(count),
             static_cast<unsigned>(MAX_TOUCH_POINTS));
    canvas.drawString(text, w / 2, 100);

    const uint16_t pointColors[] = {
        TFT_BLACK, TFT_DARKGREY, TFT_LIGHTGREY, TFT_BLACK, TFT_DARKGREY
    };

    for (uint_fast8_t i = 0; i < count; ++i) {
        char line[48];
        snprintf(line, sizeof(line), "Point %u: X:%d  Y:%d", static_cast<unsigned>(i + 1),
                 points[i].x, points[i].y);
        canvas.drawString(line, w / 2, 155 + i * 55);
        canvas.drawCircle(points[i].x, points[i].y, 24, TFT_BLACK);
        canvas.fillCircle(points[i].x, points[i].y, 8, pointColors[i]);
    }

    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawTouchPoints(nullptr, 0);
}

void loop()
{
    M5.update();
    const uint_fast8_t count = M5.Display.getTouchRaw(touchPoints, MAX_TOUCH_POINTS);

    if (touchChanged(touchPoints, count)
        && (count == 0 || millis() - lastRefresh >= 150)) {
        Serial.printf("Touch points: %u\r\n", static_cast<unsigned>(count));
        for (uint_fast8_t i = 0; i < count; ++i) {
            Serial.printf("Point %u: x:%d, y:%d\r\n", static_cast<unsigned>(i + 1),
                          touchPoints[i].x, touchPoints[i].y);
        }

        drawTouchPoints(touchPoints, count);
        saveTouchPoints(touchPoints, count);
        lastRefresh = millis();
    }
    delay(20);
}
