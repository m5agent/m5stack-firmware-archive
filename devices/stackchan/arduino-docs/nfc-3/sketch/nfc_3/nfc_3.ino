#include <M5StackChan.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <M5Utility.h>
#include <vector>

using namespace m5::nfc; // NFC common namespace
using namespace m5::nfc::a; // Use NFC-A protocol namespace (ISO 14443-3A)
using namespace m5::nfc::a::mifare; // MIFARE card common operations
using namespace m5::nfc::a::mifare::classic; // MIFARE Classic card specific operations

namespace {
auto& lcd = M5StackChan.Display();
m5::unit::UnitUnified Units; // Unit unified manager instance
m5::unit::UnitNFC unit{};  // NFC Unit instance (I2C interface)
m5::nfc::EmulationLayerA emu_a{unit}; // Create NFC-A emulation layer instance to emulate the device as an NFC tag

PICC picc{}; // Card object to emulate

// ===== Select the tag type to emulate =====
#define EMU_MIFARE_ULTRALIGHT // MIFARE Ultralight tag
// #define EMU_NTAG213  // NTAG213 tag

// ===== MIFARE Ultralight emulation data =====
#if defined(EMU_MIFARE_ULTRALIGHT)
constexpr Type type{Type::MIFARE_Ultralight};
constexpr uint8_t uid[] = {0x04, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE};// 7-byte UID (Ultralight/NTAG series uses 7-byte UID)
// Emulated tag memory data (contains NDEF message: URL https://m5stack.com/ and text "Hello M5Stack")
uint8_t picc_memory[]   = {
    0x00, 0x00, 0x00, 0x00,  // Page 0: UID bytes (to be filled by embed_uid)
    0x00, 0x00, 0x00, 0x00,  // Page 1: UID bytes (continued)
    0x00, 0xA3, 0x00, 0x00,  // Page 2: Internal data, lock bits
    0xE1, 0x10, 0x06, 0x00,  // Page 3: CC (Capability Container) - NDEF format identifier
    0x03, 0x25, 0x91, 0x01,  // Page 4: NDEF TLV start
    0x0D, 0x55, 0x04, 0x6D,  // Page 5: URI record (https://)
    0x35, 0x73, 0x74, 0x61,  // Page 6: "5sta"
    0x63, 0x6B, 0x2E, 0x63,  // Page 7: "ck.c"
    0x6F, 0x6D, 0x2F, 0x51,  // Page 8: "om/" + text record start
    0x01, 0x10, 0x54, 0x02,  // Page 9: Text record header
    0x65, 0x6E, 0x48, 0x65,  // Page 10: "enHe" (language code "en" + "He")
    0x6C, 0x6C, 0x6F, 0x20,  // Page 11: "llo "
    0x4D, 0x35, 0x53, 0x74,  // Page 12: "M5St"
    0x61, 0x63, 0x6B, 0xFE,  // Page 13: "ack" + NDEF terminator 0xFE
    0x44, 0x45, 0x46, 0x00,  // Page 14: Padding data
    0x44, 0x45, 0x46, 0x00,  // Page 15: Padding data
};
// ===== NTAG213 emulation data =====
#elif defined(EMU_NTAG213)
constexpr Type type{Type::NTAG_213};
constexpr uint8_t uid[] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33};// 7-byte UID
// Emulated tag memory data (contains multilingual NDEF message: URL + Chinese/English/Japanese text)
uint8_t picc_memory[]   = {
    0x00, 0x00, 0x00, 0x00,  // Page 0: UID bytes
    0x00, 0x00, 0x00, 0x00,  // Page 1: UID bytes (continued)
    0x00, 0x48, 0x00, 0x00,  // Page 2: Internal data, lock bits
    0xE1, 0x10, 0x12, 0x00,  // Page 3: CC (Capability Container)
    0x01, 0x03, 0xA0, 0x0C,  // Page 4: NDEF capability data
    0x34, 0x03, 0x58, 0x91,  // Page 5: NDEF TLV + message start
    0x01, 0x0D, 0x55, 0x04,  // Page 6: URI record header (https://)
    0x6D, 0x35, 0x73, 0x74,  // Page 7: "m5st"
    0x61, 0x63, 0x6B, 0x2E,  // Page 8: "ack."
    0x63, 0x6F, 0x6D, 0x2F,  // Page 9: "com/"
    0x11, 0x01, 0x11, 0x54,  // Page 10: Chinese text record header
    0x02, 0x7A, 0x68, 0xE4,  // Page 11: Language code "zh" + UTF-8 Chinese start
    0xBD, 0xA0, 0xE5, 0xA5,  // Page 12: UTF-8 encoding of "你好"
    0xBD, 0x20, 0x4D, 0x35,  // Page 13: " M5"
    0x53, 0x74, 0x61, 0x63,  // Page 14: "Stac"
    0x6B, 0x11, 0x01, 0x10,  // Page 15: "k" + English text record header
    0x54, 0x02, 0x65, 0x6E,  // Page 16: Language code "en"
    0x48, 0x65, 0x6C, 0x6C,  // Page 17: "Hell"
    0x6F, 0x20, 0x4D, 0x35,  // Page 18: "o M5"
    0x53, 0x74, 0x61, 0x63,  // Page 19: "Stac"
    0x6B, 0x51, 0x01, 0x1A,  // Page 20: "k" + Japanese text record header
    0x54, 0x02, 0x6A, 0x61,  // Page 21: Language code "ja"
    0xE3, 0x81, 0x93, 0xE3,  // Page 22: "こ" UTF-8
    0x82, 0x93, 0xE3, 0x81,  // Page 23: "ん" + start of "に"
    0xAB, 0xE3, 0x81, 0xA1,  // Page 24: "に" + "ち"
    0xE3, 0x81, 0xAF, 0x20,  // Page 25: "は "
    0x4D, 0x35, 0x53, 0x74,  // Page 26: "M5St"
    0x61, 0x63, 0x6B, 0xFE,  // Page 27: "ack" + NDEF terminator
    0x00, 0x00, 0x00, 0x00,  // Pages 28-39: Free user data area
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0x00,  //
    0x00, 0x00, 0x00, 0xBD,  // Page 40: NTAG213 configuration page
    0x02, 0x00, 0x00, 0xFF,  // Page 41: Configuration page (continued)
    0x00, 0x00, 0x00, 0x00,  // Page 42: Password protection
    0x00, 0x00, 0x00, 0x00,  // Page 43: Password acknowledgment
    0x00, 0x00, 0x00, 0x00,  // Page 44: Reserved area
};
#else
#error "Choose the target to emulate"
#endif

/**
 * @brief Calculate BCC (Block Check Character) - XOR operation on byte sequence
 * @param p    Pointer to input data
 * @param len  Data length
 * @param init Initial value (default: 0)
 * @return     BCC check value
 */
uint8_t bcc8(const uint8_t* p, const uint8_t len, const uint8_t init = 0)
{
    uint8_t v = init;
    for (uint_fast8_t i = 0; i < len; ++i) {
        v ^= p[i];
    }
    return v;
}

/**
 * @brief Correctly embed 7-byte UID into Ultralight/NTAG memory layout
 *
 * UID storage format in Ultralight/NTAG memory:
 *   Page 0: [UID0, UID1, UID2, BCC0]  BCC0 = CT ^ UID0 ^ UID1 ^ UID2
 *   Page 1: [UID3, UID4, UID5, UID6]
 *   Page 2 prefix: [BCC1]  BCC1 = UID3 ^ UID4 ^ UID5 ^ UID6
 *
 * @param mem  Target memory buffer (at least 9 bytes)
 * @param uid  7-byte UID data
 */
void embed_uid(uint8_t mem[9], const uint8_t uid[7])
{
    memcpy(mem, uid, 3);
    mem[3] = bcc8(uid, 3, 0x88 /* CT */);
    memcpy(mem + 4, uid + 3, 4);
    mem[8] = bcc8(uid + 3, 4);
}

// Color table corresponding to emulation states
constexpr uint16_t color_table[] = {
    //  None,      Off,     Idle,     Ready,   Active,      Halt };
    TFT_BLACK, TFT_RED, TFT_BLUE, TFT_YELLOW, TFT_GREEN, TFT_MAGENTA};
// Character identifiers for emulation states
//                                 None,  Off,  Idle, Ready, Active, Halt
constexpr const char* state_table[] = {"-", "O", "I", "R", "A", "H"};
}  // namespace

void setup()
{
    M5StackChan.begin();
    Serial.begin(115200);
    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    // Emulation settings
    auto cfg      = unit.config();
    cfg.emulation = true;
    cfg.mode      = NFC::A;
    unit.config(cfg);

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
    lcd.startWrite();
    lcd.fillScreen(TFT_RED);
    // Initialize emulation
    if (picc.emulate(type, uid, sizeof(uid))) {// Set card type and UID to emulate
        embed_uid(picc_memory, uid);// Embed UID into emulation memory
        // Start emulation layer with card object and memory data
        if (emu_a.begin(picc, picc_memory, sizeof(picc_memory))) {
            lcd.fillScreen(TFT_DARKGREEN);
            lcd.setTextColor(TFT_WHITE, TFT_DARKGREEN);
            lcd.setCursor(0, 16);
            // Get and display the emulated PICC info
            const auto& e_picc = emu_a.emulatePICC();
            Serial.printf("Emulation:%s %s ATQA:%04X SAK:%u\n", e_picc.typeAsString().c_str(),
                          e_picc.uidAsString().c_str(), e_picc.atqa, e_picc.sak);
            lcd.printf("%s\n%s\nATQA:%04X\nSAK:%u ", e_picc.typeAsString().c_str(), e_picc.uidAsString().c_str(),
                       e_picc.atqa, e_picc.sak);
        }
    }
    lcd.fillRect(0, 0, 32, 16, color_table[0]);
    lcd.drawString(state_table[0], 0, 0);
    lcd.endWrite();
}

void loop()
{
    M5StackChan.update();
    Units.update();// Update all registered Units
    emu_a.update();  // Update emulation layer state (MUST be called in loop)

    // Monitor emulation state changes and update screen indicator
    static EmulationLayerA::State latest{}; // Record previous state
    auto state = emu_a.state(); // Get current emulation state
    if (latest != state) {
        latest = state;
        lcd.startWrite();
        // Update top-left color block and text based on state
        lcd.fillRect(0, 0, 32, 16, color_table[m5::stl::to_underlying(state)]);
        lcd.drawString(state_table[m5::stl::to_underlying(state)], 0, 0);
        Serial.println(state_table[m5::stl::to_underlying(state)]);
        lcd.endWrite();
    }
}
