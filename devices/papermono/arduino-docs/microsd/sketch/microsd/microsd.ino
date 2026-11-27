#include <FS.h>
#include <SD_MMC.h>
#include <M5Unified.h>
#include <M5GFX.h>

M5Canvas canvas(&M5.Display);
static constexpr const char* IMAGE_PATH = "/PaperMonoTest.jpg";

void showStatus(const char* title, const char* detail)
{
    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString(title, M5.Display.width() / 2, 170);
    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.drawString(detail, M5.Display.width() / 2, 300);
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

    // M5IOE1 GPIO14 (index 13) controls the microSD power rail.
    auto& ioe1 = M5.getIOExpander(0);
    ioe1.setHighImpedance(13, false);
    ioe1.setDirection(13, true);
    ioe1.digitalWrite(13, true);
    delay(10);

    const int clk = M5.getPin(m5::pin_name_t::sd_mmc_clk);
    const int cmd = M5.getPin(m5::pin_name_t::sd_mmc_cmd);
    const int d0 = M5.getPin(m5::pin_name_t::sd_mmc_d0);
    const int d1 = M5.getPin(m5::pin_name_t::sd_mmc_d1);
    const int d2 = M5.getPin(m5::pin_name_t::sd_mmc_d2);
    const int d3 = M5.getPin(m5::pin_name_t::sd_mmc_d3);

    if (!SD_MMC.setPins(clk, cmd, d0, d1, d2, d3)) {
        showStatus("SD pin setup failed", "Check board selection");
        return;
    }
    if (!SD_MMC.begin("/sdcard", false)) {
        showStatus("SD mount failed", "Insert a formatted card");
        return;
    }

    if (!SD_MMC.exists(IMAGE_PATH)) {
        showStatus("JPG not found", IMAGE_PATH);
        return;
    }

    M5.Display.fillScreen(TFT_WHITE);
    if (!M5.Display.drawJpgFile(
            SD_MMC,
            IMAGE_PATH,
            0,
            0,
            M5.Display.width(),
            M5.Display.height(),
            0,
            0,
            0.0f,
            0.0f,
            datum_t::middle_center
        )) {
        showStatus("JPG load failed", IMAGE_PATH);
        return;
    }

    Serial.printf("Displayed: %s\n", IMAGE_PATH);
    Serial.printf("Card size: %llu MB\n", SD_MMC.cardSize() / (1024ULL * 1024ULL));
}

void loop()
{
    M5.update();
    delay(100);
}
