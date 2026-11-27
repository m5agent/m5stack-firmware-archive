#include <Arduino.h>
#include <M5Unified.h>

M5Canvas canvas(&M5.Display);

static void drawRedBorder()
{
    const int screenW = M5.Display.width();
    const int screenH = M5.Display.height();

    canvas.drawRect(0, 0, screenW, screenH, RED);
    canvas.drawRect(2, 2, screenW - 4, screenH - 4, RED);
    canvas.drawRect(4, 4, screenW - 8, screenH - 8, RED);
}

static int drawColoredWordCentered(const char* word, int y, const uint16_t* colors, int colorCount)
{
    const int screenW = M5.Display.width();
    const int len     = strlen(word);
    int totalW        = 0;

    for (int i = 0; i < len; ++i) {
        totalW += canvas.textWidth(String(word[i]));
    }

    int x = (screenW - totalW) / 2;
    for (int i = 0; i < len; ++i) {
        canvas.setTextColor(colors[i % colorCount]);
        String ch(word[i]);
        canvas.drawString(ch, x, y);
        x += canvas.textWidth(ch);
    }
    return x;
}

static void drawMainDemo()
{
    const int screenW = M5.Display.width();
    const int screenH = M5.Display.height();

    static const uint16_t paperColors[1] = {BLACK};
    static const uint16_t colorColors[5] = {RED, YELLOW, GREEN, BLUE, RED};
    static const uint16_t e6Colors[6]    = {WHITE, BLACK, RED, YELLOW, GREEN, BLUE};

    canvas.fillSprite(WHITE);
    drawRedBorder();

    canvas.setTextDatum(top_left);
    canvas.setTextSize(1);
    canvas.setFont(&fonts::FreeSansBold24pt7b);

    drawColoredWordCentered("PAPER", 168, paperColors, 1);
    drawColoredWordCentered("COLOR", 236, colorColors, 5);

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.setTextColor(BLACK);
    canvas.setTextDatum(middle_center);
    canvas.drawString("Panel: ED2208-DOA", screenW / 2, 336);

    char resolutionBuf[32];
    snprintf(resolutionBuf, sizeof(resolutionBuf), "Resolution: %d x %d", screenW, screenH);
    canvas.setTextColor(BLUE);
    canvas.drawString(resolutionBuf, screenW / 2, 376);

    canvas.setFont(&fonts::Font4);
    canvas.setTextColor(BLACK);
    canvas.drawString("4\" E-Paper E6 Full-Color", screenW / 2, 418);

    const int barX  = 46;
    const int barY  = 448;
    const int barW  = screenW - 92;
    const int barH  = 34;
    const int cellW = barW / 6;
    for (int i = 0; i < 6; ++i) {
        const int x = barX + i * cellW;
        const int w = (i == 5) ? (barX + barW - x) : cellW;
        canvas.fillRect(x, barY, w, barH, e6Colors[i]);
        canvas.drawRect(x, barY, w, barH, BLACK);
    }

    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg          = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);

    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    M5.Display.setRotation(0);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawMainDemo();
}

void loop()
{
    M5.update();
    delay(100);
}
