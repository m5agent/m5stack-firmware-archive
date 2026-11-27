#include <Arduino.h>
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <wiring/m5_unit_unified_wiring.hpp>

namespace {

using m5::nfc::NFC;
using m5::nfc::EmulationLayerA;
using m5::nfc::a::PICC;
using m5::nfc::a::Type;

constexpr uint16_t kBlack = 0x0000;
constexpr uint16_t kDark = 0x52AA;
constexpr uint16_t kWhite = 0xFFFF;
constexpr uint8_t kFrontlightBrightness = 160;
constexpr int kNfcIrqPin = 6;

// Seven-byte NFC-A UID used by this demo.
constexpr uint8_t kUid[] = {0x04, 0x50, 0x41, 0x50, 0x45, 0x52, 0x01};

// MIFARE Ultralight memory. Page 4 contains an NDEF Text record whose
// language is "en" and whose text is exactly "PaperMONO".
uint8_t cardMemory[64] = {
    0x00, 0x00, 0x00, 0x00,  // UID0, UID1, UID2, BCC0
    0x00, 0x00, 0x00, 0x00,  // UID3, UID4, UID5, UID6
    0x00, 0xA3, 0x00, 0x00,  // BCC1, internal, lock bytes
    0xE1, 0x10, 0x06, 0x00,  // Capability Container
    0x03, 0x10, 0xD1, 0x01,  // NDEF TLV and record header
    0x0C, 0x54, 0x02, 0x65,  // payload length, Text type, language length, 'e'
    0x6E, 0x50, 0x61, 0x70,  // 'nPaper'
    0x65, 0x72, 0x4D, 0x4F,  // 'erMO'
    0x4E, 0x4F, 0xFE, 0x00,  // 'NO', terminator
};

m5::unit::UnitUnified units;
m5::unit::UnitNFC nfc;
EmulationLayerA emulation(nfc);
PICC card;
M5Canvas canvas(&M5.Display);

uint8_t calculateBcc(const uint8_t* data, size_t length,
                     uint8_t initial = 0)
{
    uint8_t value = initial;
    for (size_t i = 0; i < length; ++i) {
        value ^= data[i];
    }
    return value;
}

void embedUid()
{
    memcpy(cardMemory, kUid, 3);
    cardMemory[3] = calculateBcc(kUid, 3, 0x88);
    memcpy(cardMemory + 4, kUid + 3, 4);
    cardMemory[8] = calculateBcc(kUid + 3, 4);
}

void drawNfcMark(int cx, int cy)
{
    canvas.drawCircle(cx, cy, 78, kBlack);
    canvas.drawCircle(cx, cy, 69, kBlack);
    canvas.fillCircle(cx, cy, 8, kBlack);
    canvas.drawArc(cx, cy, 31, 25, 305, 55, kBlack);
    canvas.drawArc(cx, cy, 49, 43, 305, 55, kBlack);
    canvas.drawArc(cx, cy, 67, 61, 305, 55, kBlack);
}

void drawReadyScreen(bool ready)
{
    M5.Display.setEpdMode(epd_mode_t::epd_fast);
    canvas.fillSprite(kWhite);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(kBlack, kWhite);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.drawString("NFC CARD", 240, 100);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(ready ? kBlack : kDark, kWhite);
    canvas.drawString(ready ? "EMULATION READY" : "INITIALIZATION FAILED",
                      240, 170);

    drawNfcMark(240, 320);

    canvas.fillRoundRect(34, 450, 412, 176, 8, kBlack);
    canvas.setTextColor(kWhite, kBlack);
    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.drawString("MIFARE ULTRALIGHT", 240, 495);
    canvas.setFont(&fonts::FreeSans9pt7b);
    canvas.drawString("UID  04 50 41 50 45 52 01", 240, 548);
    canvas.drawString("NDEF TEXT  PaperMONO", 240, 590);

    canvas.setTextColor(kDark, kWhite);
    canvas.drawString(ready ? "Hold near an NFC reader"
                            : "Check serial output",
                      240, 700);
    canvas.pushSprite(0, 0);
}

bool beginCardEmulation()
{
    auto config = nfc.config();
    config.emulation = true;
    config.mode = NFC::A;
    config.using_irq = true;
    config.irq = kNfcIrqPin;
    nfc.config(config);

    if (!m5::unit::wiring::i2cClass(units, nfc, M5.In_I2C) ||
        !units.begin()) {
        return false;
    }

    embedUid();
    return card.emulate(Type::MIFARE_Ultralight, kUid, sizeof(kUid)) &&
           emulation.begin(card, cardMemory, sizeof(cardMemory));
}

const char* stateName(EmulationLayerA::State state)
{
    static constexpr const char* names[] = {
        "None", "Off", "Idle", "Ready", "Active", "Halt"
    };
    return names[static_cast<unsigned>(state)];
}

}  // namespace

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("PaperMono NFC-A card emulation demo");

    auto config = M5.config();
    config.clear_display = false;
    config.internal_rtc = false;
    config.internal_imu = false;
    config.internal_mic = false;
    M5.begin(config);

    M5.Display.setRotation(0);
    M5.Display.setBrightness(kFrontlightBrightness);
    canvas.setColorDepth(16);
    if (!canvas.createSprite(M5.Display.width(), M5.Display.height())) {
        Serial.println("Display canvas allocation failed");
        return;
    }

    const bool ready = beginCardEmulation();
    drawReadyScreen(ready);
    if (!ready) {
        Serial.println("NFC-A emulation initialization failed");
        return;
    }

    const auto& picc = emulation.emulatePICC();
    Serial.printf("NFC-A emulation ready: %s\n",
                  picc.typeAsString().c_str());
    Serial.printf("UID: %s, ATQA: %04X, SAK: %u\n",
                  picc.uidAsString().c_str(), picc.atqa, picc.sak);
    Serial.println("NDEF text: PaperMONO");
}

void loop()
{
    // Keep the loop short because NFC-A response timing is strict.
    units.update();
    emulation.update();

    static EmulationLayerA::State previous = EmulationLayerA::State::None;
    const auto state = emulation.state();
    if (state != previous) {
        Serial.printf("NFC-A state: %s\n", stateName(state));
        previous = state;
    }
    yield();
}
