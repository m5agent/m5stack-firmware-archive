#include <Arduino.h>
#include <M5Unified.h>

M5Canvas canvas(&M5.Display);

static void drawBlackBorder()
{
    const int screenW = M5.Display.width();
    const int screenH = M5.Display.height();

    canvas.drawRect(0, 0, screenW, screenH, TFT_BLACK);
    canvas.drawRect(2, 2, screenW - 4, screenH - 4, TFT_BLACK);
    canvas.drawRect(4, 4, screenW - 8, screenH - 8, TFT_BLACK);
}

static int drawGrayscaleWordCentered(const char* word, int y, const uint16_t* colors, int colorCount)
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

    static const uint16_t paperColors[1] = {TFT_BLACK};
    static const uint16_t monoColors[4] = {
        TFT_BLACK,
        TFT_RED,
        TFT_DARKGREY,        
        TFT_BLACK
    };
    static const uint16_t grayColors[4] = {
        TFT_BLACK,
        TFT_RED,        
        TFT_DARKGREY,
        TFT_WHITE
    };

    canvas.fillSprite(TFT_WHITE);
    drawBlackBorder();

    canvas.setTextDatum(top_left);
    canvas.setTextSize(1);
    canvas.setFont(&fonts::FreeSansBold24pt7b);

    drawGrayscaleWordCentered("PAPER", 168, paperColors, 1);
    drawGrayscaleWordCentered("MONO", 236, monoColors, 4);

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.setTextDatum(middle_center);
    canvas.drawString("Panel: SSD1677 + FT6336G", screenW / 2, 336);

    char resolutionBuf[32];
    snprintf(
        resolutionBuf,
        sizeof(resolutionBuf),
        "Resolution: %d x %d",
        screenW,
        screenH
    );
    canvas.drawString(resolutionBuf, screenW / 2, 376);

    canvas.drawString(
        "4-Level Grayscale E-Paper",
        screenW / 2,
        418
    );

    const int barX  = 46;
    const int barY  = 448;
    const int barW  = screenW - 92;
    const int barH  = 46;
    const int cellW = barW / 4;

    for (int i = 0; i < 4; ++i) {
        const int x = barX + i * cellW;
        const int w = (i == 3) ? (barX + barW - x) : cellW;

        canvas.fillRect(x, barY, w, barH, grayColors[i]);
        canvas.drawRect(x, barY, w, barH, TFT_BLACK);
    }

    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);

    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawMainDemo();
}

void loop()
{
    M5.update();
    delay(100);
}
