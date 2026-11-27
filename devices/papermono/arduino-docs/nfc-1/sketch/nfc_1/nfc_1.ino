#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <utility/M5IOE1_Class.hpp>
#include <vector>

using namespace m5::nfc::a;

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitNFC unit{};
m5::nfc::NFCLayerA nfc_a{unit};

constexpr uint8_t NFC_IRQ_PIN = 6;
constexpr auto NFC_CONTROL_PIN = m5::M5IOE1_Class::gpio4;
constexpr int STATUS_HEIGHT = 92;
}  // namespace

void drawBottomStatus(const char* message)
{
    const int top = lcd.height() - STATUS_HEIGHT;
    lcd.fillRect(0, top, lcd.width(), STATUS_HEIGHT, TFT_WHITE);
    lcd.drawFastHLine(0, top, lcd.width(), TFT_BLACK);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(middle_center);
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.drawString(message, lcd.width() / 2, top + STATUS_HEIGHT / 2);
}

void stopOnError(const char* message)
{
    M5_LOGE("%s", message);
    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(middle_center);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, lcd.height() / 2 - 48);
    lcd.drawString(message, lcd.width() / 2, lcd.height() / 2 + 40);
    lcd.display();
    while (true) {
        delay(1000);
    }
}

void drawReadyScreen()
{
    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(middle_center);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, 90);
    lcd.drawString("Quick Scan", lcd.width() / 2, 170);
    lcd.drawString("Place NFC-A tag", lcd.width() / 2, 300);
    drawBottomStatus("Waiting for NFC-A tag");
    lcd.display();
}

void setup()
{
    Serial.begin(115200);

    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    lcd.setRotation(0);
    lcd.setEpdMode(epd_mode_t::epd_text);

    auto& ioe = M5.getIOExpander(0);
    ioe.setHighImpedance(NFC_CONTROL_PIN, false);
    ioe.setDirection(NFC_CONTROL_PIN, true);
    ioe.digitalWrite(NFC_CONTROL_PIN, true);
    delay(10);

    auto nfc_cfg = unit.config();
    nfc_cfg.using_irq = true;
    nfc_cfg.irq = NFC_IRQ_PIN;
    unit.config(nfc_cfg);

    if (!(Units.add(unit, M5.In_I2C) && Units.begin())) {
        stopOnError("NFC init failed");
    }

    M5_LOGI("M5Unit-NFC initialized");
    M5_LOGI("%s", Units.debugInfo().c_str());
    drawReadyScreen();
}

void loop()
{
    M5.update();
    Units.update();

    static bool tagWasShown = false;
    std::vector<PICC> piccs;
    if (!nfc_a.detect(piccs)) {
        tagWasShown = false;
        delay(20);
        return;
    }
    if (tagWasShown) {
        nfc_a.deactivate();
        delay(100);
        return;
    }
    tagWasShown = true;

    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(top_center);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, 24);
    lcd.drawString("Quick Scan Result", lcd.width() / 2, 76);
    lcd.setFont(&fonts::FreeSansBold12pt7b);

    uint16_t identified = 0;
    int y = 150;
    for (auto& picc : piccs) {
        if (!nfc_a.identify(picc)) {
            M5_LOGW("Failed to identify %s", picc.uidAsString().c_str());
            continue;
        }

        M5.Log.printf("PICC:%s %s %04X/%02X %u/%u\n", picc.uidAsString().c_str(),
                      picc.typeAsString().c_str(), picc.atqa, picc.sak, picc.userAreaSize(),
                      picc.totalSize());

        if (identified < 5) {
            char uidLine[64];
            char infoLine[96];
            snprintf(uidLine, sizeof(uidLine), "[%u] UID: %s", identified, picc.uidAsString().c_str());
            snprintf(infoLine, sizeof(infoLine), "%s  %04X/%02X  %u/%u", picc.typeAsString().c_str(),
                     picc.atqa, picc.sak, picc.userAreaSize(), picc.totalSize());
            lcd.drawString(uidLine, lcd.width() / 2, y);
            lcd.drawString(infoLine, lcd.width() / 2, y + 38);
            y += 92;
        }
        ++identified;
    }

    char countLine[32];
    snprintf(countLine, sizeof(countLine), "Found: %u tag(s)", identified);
    drawBottomStatus(countLine);
    nfc_a.deactivate();
    lcd.display();
    M5.Log.printf("==> %u PICC\n", identified);
    delay(500);
}
