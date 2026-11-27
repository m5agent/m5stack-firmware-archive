#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <M5Utility.h>
#include <Wire.h>
#include <vector>

using namespace m5::nfc::a; // NFC-A protocol layer
using namespace m5::nfc::a::mifare; // MIFARE card common operations
using namespace m5::nfc::a::mifare::classic; // MIFARE Classic card specific operations

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units; // Unit unified manager instance
m5::unit::UnitNFC unit{};  // NFC Unit instance (I2C interface)
m5::nfc::NFCLayerA nfc_a{unit}; // NFC-A protocol layer instance for ISO 14443-3A cards

// KeyA that can authenticate all blocks
// If it's a different key value, change it
constexpr Key keyA = DEFAULT_KEY;  // Default as 0xFFFFFFFFFFFF
}  // namespace

void setup()
{
    M5.begin();

    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    bool unit_ready{};// Unit initialization status flag

    // Add NFC Unit to manager and initialize
    unit_ready = Units.add(unit, M5.In_I2C) && Units.begin();
    if (!unit_ready) {
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }

    M5_LOGI("M5UnitUnified initialized");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.setFont(&fonts::FreeMono9pt7b);
    lcd.fillScreen(0);
    lcd.setCursor(0, 0);
    lcd.printf("Please put the PICC\nand click\nBtnA");
    M5.Log.printf("Please put the PICC and click BtnA\n");
}

void loop()
{
    M5.update();
    Units.update();// Update all registered Units

    if (M5.BtnA.wasClicked()) {
        lcd.fillScreen(0);
        lcd.setCursor(0, 0);
        PICC picc{}; // Create card object
        if (nfc_a.detect(picc)) { // Detect a single card
            // Identify card type and reactivate (get full communication parameters)
            if (nfc_a.identify(picc) && nfc_a.reactivate(picc)) {
                lcd.printf("%s\n%s", picc.uidAsString().c_str(), picc.typeAsString().c_str());
                // Print detailed info: UID, type, user area size, total size
                M5.Log.printf("==== Dump %s %s %u/%u ====\n", picc.uidAsString().c_str(), picc.typeAsString().c_str(),
                              picc.userAreaSize(), picc.totalSize());
                // Dump all card data (needs key for MIFARE Classic, key parameter ignored for other types)
                nfc_a.dump(keyA);  // Need key if MIFARE classic, Ignore key if not MIFARE classic
                nfc_a.deactivate();
            } else {
                lcd.printf("Failed to identify");
                M5_LOGE("Failed to identify/activate %s", picc.uidAsString().c_str());
            }
        } else {
            lcd.printf("PICC NOT exists");
            M5.Log.printf("PICC NOT exists\n");
        }
    }
}
