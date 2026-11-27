#include <M5Unified.h>

M5Canvas canvas(&M5.Display);

struct ModeOption {
    const char* name;
    epd_mode_t mode;
};

const ModeOption modeOptions[] = {
    {"epd_quality", epd_mode_t::epd_quality},
    {"epd_text", epd_mode_t::epd_text},
    {"epd_fast", epd_mode_t::epd_fast},
    {"epd_fastest", epd_mode_t::epd_fastest},
};

constexpr std::size_t MODE_COUNT = sizeof(modeOptions) / sizeof(modeOptions[0]);
std::size_t selectedMode = 0;
uint32_t refreshCount = 0;

void drawScreen()
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_BLACK);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.drawString("E-Paper Refresh Mode", w / 2, 90);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    char selectedText[48];
    snprintf(selectedText, sizeof(selectedText), "Selected: %s", modeOptions[selectedMode].name);
    canvas.drawString(selectedText, w / 2, 175);
    canvas.drawString("BtnA: previous   BtnB: next", w / 2, 245);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    for (std::size_t i = 0; i < MODE_COUNT; ++i) {
        const int y = 315 + static_cast<int>(i) * 42;
        const bool selected = i == selectedMode;
        canvas.setTextColor(selected ? TFT_BLACK : TFT_DARKGREY);
        canvas.drawString(selected ? ">" : " ", w / 2 - 180, y);
        canvas.drawString(modeOptions[i].name, w / 2, y);
    }

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    char countText[32];
    snprintf(countText, sizeof(countText), "Refresh count: %lu", (unsigned long)refreshCount);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString(countText, w / 2, h - 35);
    canvas.pushSprite(0, 0);
}

void selectMode(std::size_t index)
{
    selectedMode = index % MODE_COUNT;
    M5.Display.setEpdMode(modeOptions[selectedMode].mode);
    ++refreshCount;
    drawScreen();
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setEpdMode(modeOptions[selectedMode].mode);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawScreen();
}

void loop()
{
    M5.update();

    if (M5.BtnA.wasPressed()) {
        selectMode(selectedMode + MODE_COUNT - 1);
    }
    if (M5.BtnB.wasPressed()) {
        selectMode(selectedMode + 1);
    }

    delay(20);
}
