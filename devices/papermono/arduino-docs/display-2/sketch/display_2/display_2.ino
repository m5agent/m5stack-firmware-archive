#include <M5Unified.h>
#include <M5GFX.h>

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
constexpr int BUTTON_X = 16;
constexpr int BUTTON_Y = 130;
constexpr int BUTTON_H = 90;
constexpr int BUTTON_GAP = 34;

std::size_t selectedMode = 0;
uint32_t refreshCount = 0;
LGFX_Button modeButtons[MODE_COUNT];

int buttonWidth()
{
    return M5.Display.width() - BUTTON_X * 2;
}

void initModeButtons()
{
    const int width = buttonWidth();
    for (std::size_t i = 0; i < MODE_COUNT; ++i) {
        const int y = BUTTON_Y + static_cast<int>(i) * (BUTTON_H + BUTTON_GAP);
        modeButtons[i].initButtonUL(&M5.Lcd, BUTTON_X, y, width, BUTTON_H,
                                    TFT_BLACK, TFT_WHITE, TFT_BLACK,
                                    modeOptions[i].name, 1, 1);
    }
}

void drawScreen()
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);

    char selectedText[40];
    snprintf(selectedText, sizeof(selectedText), "Selected: %s", modeOptions[selectedMode].name);
    canvas.drawString(selectedText, w / 2, 30);

    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.drawString("Tap a button to refresh once", w / 2, 82);

    canvas.setFont(&fonts::FreeSansBold18pt7b);

    for (std::size_t i = 0; i < MODE_COUNT; ++i) {
        const int y = BUTTON_Y + static_cast<int>(i) * (BUTTON_H + BUTTON_GAP);
        const bool selected = i == selectedMode;
        const int width = buttonWidth();

        if (selected) {
            canvas.fillRect(BUTTON_X, y, width, BUTTON_H, TFT_BLACK);
        } else {
            canvas.fillRect(BUTTON_X, y, width, BUTTON_H, TFT_WHITE);
        }
        canvas.drawRect(BUTTON_X, y, width, BUTTON_H, TFT_BLACK);
        canvas.setTextColor(selected ? TFT_WHITE : TFT_BLACK);
        canvas.drawString(modeOptions[i].name, w / 2, y + BUTTON_H / 2);
    }

    char countText[32];
    snprintf(countText, sizeof(countText), "Refresh count: %lu", (unsigned long)refreshCount);
    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString(countText, w / 2, h - 110);

    const int stripeY = h - 75;
    const int stripeW = (w - 48) / 4;
    const uint16_t stripeColors[] = {TFT_BLACK, TFT_DARKGREY, TFT_LIGHTGREY, TFT_WHITE};
    for (int i = 0; i < 4; ++i) {
        const int x = 24 + i * stripeW;
        canvas.fillRect(x, stripeY, stripeW, 42, stripeColors[i]);
        canvas.drawRect(x, stripeY, stripeW, 42, TFT_BLACK);
    }

    canvas.pushSprite(0, 0);
}

void selectMode(std::size_t index)
{
    selectedMode = index;
    M5.Display.setEpdMode(modeOptions[selectedMode].mode);
    ++refreshCount;
    drawScreen();
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    M5.Display.setRotation(0);
    M5.Display.setEpdMode(modeOptions[selectedMode].mode);

    initModeButtons();
    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawScreen();
}

void loop()
{
    M5.update();
    m5::touch_point_t touchPoint;
    const bool touched = M5.Display.getTouchRaw(&touchPoint, 1) > 0;

    for (std::size_t i = 0; i < MODE_COUNT; ++i) {
        const bool inside = touched && modeButtons[i].contains(touchPoint.x, touchPoint.y);
        modeButtons[i].press(inside);
        if (modeButtons[i].justPressed()) {
            selectMode(i);
        }
    }
    delay(5);
}
