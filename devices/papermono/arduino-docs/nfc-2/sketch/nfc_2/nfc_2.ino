#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <utility/M5IOE1_Class.hpp>

using namespace m5::nfc::a;
using namespace m5::nfc::a::mifare;
using namespace m5::nfc::a::mifare::classic;

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitNFC unit{};
m5::nfc::NFCLayerA nfc_a{unit};

constexpr uint8_t NFC_IRQ_PIN = 6;
constexpr auto NFC_CONTROL_PIN = m5::M5IOE1_Class::gpio4;
constexpr Key keyA = DEFAULT_KEY;
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
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, 110);
    lcd.drawString("Complete Reading", lcd.width() / 2, 190);
    lcd.drawString("Place tag and press BtnA", lcd.width() / 2, 340);
    drawBottomStatus("BtnA: scan and dump tag");
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

    if (!M5.BtnA.wasClicked()) {
        delay(20);
        return;
    }

    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(middle_center);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, 80);
    lcd.drawString("Reading...", lcd.width() / 2, 180);
    drawBottomStatus("Waiting for tag...");
    lcd.display();

    PICC picc{};
    if (!nfc_a.detect(picc)) {
        drawBottomStatus("PICC not found");
        lcd.display();
        M5.Log.printf("PICC NOT exists\n");
        delay(500);
        drawReadyScreen();
        return;
    }

    if (!nfc_a.identify(picc) || !nfc_a.reactivate(picc)) {
        drawBottomStatus("Identify failed");
        lcd.display();
        M5_LOGE("Failed to identify/activate %s", picc.uidAsString().c_str());
        nfc_a.deactivate();
        delay(500);
        drawReadyScreen();
        return;
    }

    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(top_center);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, 30);
    lcd.drawString("Complete Reading", lcd.width() / 2, 86);
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.drawString(picc.uidAsString().c_str(), lcd.width() / 2, 190);
    lcd.drawString(picc.typeAsString().c_str(), lcd.width() / 2, 240);
    drawBottomStatus("Dumping data to Serial...");
    lcd.display();

    M5.Log.printf("==== Dump %s %s %u/%u ====\n", picc.uidAsString().c_str(),
                  picc.typeAsString().c_str(), picc.userAreaSize(), picc.totalSize());
    nfc_a.dump(keyA);
    M5.Log.printf("==== Dump complete ====\n");

    drawBottomStatus("Dump complete");
    lcd.display();
    nfc_a.deactivate();
    delay(800);
    drawReadyScreen();
}
