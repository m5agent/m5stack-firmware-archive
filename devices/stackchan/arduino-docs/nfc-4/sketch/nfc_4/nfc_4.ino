#include <M5StackChan.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <M5Utility.h>
#include <vector>

using namespace m5::nfc; // NFC common namespace
using namespace m5::nfc::a; // Use NFC-A protocol namespace (ISO 14443-3A)
using namespace m5::nfc::a::mifare; // MIFARE card common operations

namespace {
auto& lcd = M5StackChan.Display();
m5::unit::UnitUnified Units; // Unit unified manager instance
m5::unit::UnitNFC unit{};  // NFC Unit instance (I2C interface)
m5::nfc::NFCLayerA nfc_a{unit};// NFC-A protocol layer instance

// Classic default KeyA (0xFFFFFFFFFFFF)
// If your card uses a different key, change it here
constexpr classic::Key keyA = classic::DEFAULT_KEY;

// Test message strings (selected based on card capacity)
constexpr char long_msg[]  = "This is a sample message buffer used for testing NFC page writes and data integrity verification purposes.";// For large-capacity cards (user area >= 120 bytes)
constexpr char short_msg[] = "0123456789ABCDEFGHIJ";// For small-capacity cards (user area < 120 bytes)

/**
 * @brief Cross-block continuous read/write test (triggered by click)
 *
 * Write test message to card starting from specified block, read back and verify data integrity,
 * then clear by writing all zeros.
 * Uses high-level read()/write() API which handles cross-block/cross-sector operations automatically.
 *
 * Flow: Write -> Dump -> Read back & Verify -> Clear -> Dump
 *
 * @param sblock  Starting block number to write
 * @param msg     Test message string to write
 * @return true if all operations (write, verify, clear) succeeded
 */
bool read_write(const uint8_t sblock, const char* msg)
{
    auto len = strlen(msg);
    uint8_t buf[(strlen(msg) + 15) / 16 * 16]{};  // Round up to 16-byte alignment (Classic block size)
    uint16_t rx_len = sizeof(buf);

    // Write test message to card
    M5.Log.printf("================================ WRITE block:%u len:%zu\n", sblock, sizeof(buf));
    if (!nfc_a.write(sblock, (const uint8_t*)msg, len, keyA)) {
        M5_LOGE("Failed to write block %u", sblock);
        return false;
    }
    lcd.fillScreen(TFT_ORANGE);

    // Dump written data for visual confirmation
    nfc_a.mifareClassicAuthenticateA(classic::get_sector_trailer_block(sblock), keyA);// Authenticate sector before dump
    nfc_a.dump(sblock);

    // Read back and verify data integrity
    if (!nfc_a.read(buf, rx_len, sblock, keyA)) {
        M5_LOGE("Failed to read");
        return false;
    }
    lcd.fillScreen(TFT_BLUE);

    bool verify_ok = (memcmp(buf, msg, len) == 0);// Compare read data with original message
    M5.Log.printf("================================ VERIFY:%s\n", verify_ok ? "OK" : "NG");
    if (!verify_ok) {
        M5_LOGE("VERIFY NG!!");
        m5::utility::log::dump(buf, rx_len, false);// Dump read data for debugging
    }

    // Clear by writing all zeros
    memset(buf, 0, sizeof(buf));
    lcd.fillScreen(TFT_MAGENTA);
    if (!nfc_a.write(sblock, buf, sizeof(buf), keyA)) {
        M5_LOGE("Failed to clear");
        return false;
    }
    M5.Log.printf("================================ CLEAR\n");

    // Dump cleared data for visual confirmation
    nfc_a.mifareClassicAuthenticateA(classic::get_sector_trailer_block(sblock), keyA);
    nfc_a.dump(sblock);

    return true;
}

/**
 * @brief Single block read/write test
 *
 * Write a fixed test string to a single 16-byte block using low-level read16()/write16() API,
 * read back and verify, then clear.
 * Unlike read_write(), this operates on exactly one block without cross-sector handling.
 *
 * Flow: Authenticate -> Dump before -> Write -> Dump after -> Read & Verify -> Clear -> Dump
 *
 * @param block  Block number to read/write (must NOT be a sector trailer block)
 */
void read_write_single_block(const uint8_t block)
{
    constexpr char msg[] = "M5Unit-RFID";// Fixed test message (fits within 16-byte block)

    // Authenticate with KeyA before any read/write operation
    if (!nfc_a.mifareClassicAuthenticateA(block, keyA)) {
        M5_LOGE("Failed to AuthA");
        return;
    }

    // Dump block content before write
    M5.Log.printf("Before[%u] ----\n", block);
    nfc_a.dump(block);

    // Write test message to the block
    M5.Log.printf("Write\n");
    if (!nfc_a.write16(block, (const uint8_t*)msg, sizeof(msg))) {
        M5_LOGE("Failed to write");
        return;
    }

    // Dump block content after write
    M5.Log.printf("After[%u] ----\n", block);
    nfc_a.dump(block);

    // Read back and verify data integrity
    uint8_t rbuf[16]{};
    if (!nfc_a.read16(rbuf, block)) {
        M5_LOGE("Failed to read");
        return;
    }
    bool verify = (std::memcmp(rbuf, (const uint8_t*)msg, sizeof(msg)) == 0);// Compare read data with original
    M5.Log.printf("Verify %s\n", verify ? "OK" : "NG");

    // Clear block by writing minimal zero data (library pads to 16 bytes)
    M5.Log.printf("Clear\n");
    uint8_t c[1]{};
    if (!nfc_a.write16(block, c, sizeof(c))) {
        M5_LOGE("Failed to write");
        return;
    }

    // Dump block content after clear
    nfc_a.dump(block);
}

}  // namespace

void setup()
{
    M5StackChan.begin();
    Serial.begin(115200);
    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    bool unit_ready{};// Unit initialization status flag

    // Add NFC Unit to manager and initialize
    unit_ready = Units.add(unit, M5.In_I2C) && Units.begin();
    if (!unit_ready) {
        // Initialization failed: turn screen red and enter infinite loop
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
    M5_LOGI("M5UnitUnified initialized");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.setFont(&fonts::FreeMonoBold9pt7b);
    lcd.setCursor(0, 0);
    lcd.printf("Put Classic card\nand touch/hold screen");
    M5.Log.printf("Put Classic card and touch/hold screen\n");
}

void loop()
{
    M5StackChan.update();
    Units.update();// Update all registered Units

    bool clicked = M5.Touch.getDetail().wasClicked();  // For cross-block read/write test
    bool held    = M5.Touch.getDetail().wasHold();     // For single block read/write test

    if (clicked || held) {
        PICC picc;
        if (nfc_a.detect(picc)) {
            lcd.fillScreen(TFT_DARKGREEN);

            if (nfc_a.identify(picc) && nfc_a.reactivate(picc)) {
                // Print card information: UID, type, user area size, total size
                M5.Log.printf("PICC:%s %s %u/%u\n",
                              picc.uidAsString().c_str(),
                              picc.typeAsString().c_str(),
                              picc.userAreaSize(),
                              picc.totalSize());

                // Only process MIFARE Classic cards, skip all other types
                if (!picc.isMifareClassic()) {
                    M5.Log.printf("Not a MIFARE Classic card, skipped\n");
                } else if (clicked) {
                    // Cross-block continuous read/write test
                    M5.Speaker.tone(2000, 30);
                    // Select message based on card capacity
                    const char* msg = (picc.userAreaSize() >= 120) ? long_msg : short_msg;
                    bool ret = read_write(picc.firstUserBlock(), msg);// Start from first user block
                    lcd.fillScreen(ret ? TFT_BLACK : TFT_RED);// Black = success, Red = failure

                } else if (held) {
                    // Single block read/write test
                    M5.Speaker.tone(4000, 30);
                    // Use second-to-last block (avoid sector trailer which contains keys and access bits)
                    read_write_single_block(picc.blocks - 2);
                }

                nfc_a.deactivate();// Release card communication
            } else {
                M5_LOGE("Failed to identify/activate");
            }
        } else {
            M5.Log.printf("PICC NOT detected\n");
        }

        lcd.setCursor(0, 0);
        lcd.printf("Put Classic card\nand touch/hold screen");
        M5.Log.printf("Put Classic card and touch/hold screen\n");
    }
}
