#include <M5StackChan.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <M5Utility.h>
#include <vector>
using namespace m5::nfc::a;// NFC-A protocol layer
using namespace m5::nfc::a::mifare;// MIFARE card common operations
using namespace m5::nfc::a::mifare::classic;// MIFARE Classic card specific operations
namespace {
auto& lcd = M5StackChan.Display();
m5::unit::UnitUnified Units; // Unit unified manager instance
m5::unit::UnitNFC unit{};  // NFC Unit instance (I2C interface)
m5::nfc::NFCLayerA nfc_a{unit};// NFC-A protocol layer instance
// KeyA,B that can authenticate all blocks
// If it's a different key value, change it
constexpr Key keyA = DEFAULT_KEY;  // Default as 0xFFFFFFFFFFFF
constexpr Key keyB = DEFAULT_KEY;  // Default as 0xFFFFFFFFFFFF
/* @brief Non-rechargeable e-wallet demonstration: create and use value block
*
* This function demonstrates how to create a non-rechargeable value block on a MIFARE Classic card and perform decrement operations.
* Main steps include:
*   1. Authenticate sector
*   2. Set block access to read/write
*   3. Initialize value block with initial amount
*   4. Change access to non-rechargeable mode
*   5. Demonstrate decrementing the amount
*   6. Attempt to recharge (should fail)
*   7. Demonstrate copying value block
*   8. Finally restore block to normal read/write
*
* @param block Block number, must be user block and not sector trailer
* @param akey  KeyA for authentication
* @param bkey  KeyB for modifying access conditions
*/
void non_rechargeable_value_block(const uint8_t block, const Key& akey, const Key& bkey)
{
    auto& picc = nfc_a.activatedPICC();// Get the currently activated PICC object
    // Verify that block and block-1 are both user blocks (not config or sector trailer)
    if (!picc.isUserBlock(block) || !picc.isUserBlock(block - 1)) {
        M5_LOGE("block and block - 1 must be user block %u %u", block, block - 1);
        return;
    }
    // Step 1: Authenticate sector with KeyA
    if (!nfc_a.mifareClassicAuthenticateA(block, akey)) {
        M5_LOGE("Failed to AUTH A %u/%u", block, block);
        return;
    }
    // Change read/write block
    if (!nfc_a.mifareClassicWriteAccessCondition(block, READ_WRITE_BLOCK, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition %u", block);
        return;
    }
    // Write initial value
    if (!nfc_a.mifareClassicWriteValueBlock(block, 1234567)) {
        M5_LOGE("Failed to WriteValue %u", block);
        return;
    }
    // After writing the value, change it to the value block (Non rechargeable)
    if (!nfc_a.mifareClassicWriteAccessCondition(block, VALUE_BLOCK_NON_RECHARGEABLE, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition %u", block);
        return;
    }
    M5.Log.printf("==== Initial value\n");
    nfc_a.dump(block);
    // Decrement and transfer value
    if (!nfc_a.mifareClassicDecrementValueBlock(block, 4567u)) {
        M5_LOGE("Failed to decrement %u", block);
        return;
    }
    M5.Log.printf("==== Decrement done\n");
    nfc_a.dump(block);
    // Incremental operations cannot be performed because charging is not possible
    if (nfc_a.mifareClassicIncrementValueBlock(block, 9876543)) {
        M5_LOGE("Oops!?!?");
        return;
    } else {
        // Passing through this block is normal
        M5.Log.printf("Incremental operations cannot be performed because charging is not possible\n");
        // The Increment command failed, causing a HALT, so need reactivate and auth
        if (!nfc_a.reactivate()) {
            M5_LOGE("Failed to reactivate");
            return;
        }
        if (!nfc_a.mifareClassicAuthenticateA(block, akey)) {
            M5_LOGE("Failed to AUTH %u/%u", block, block);
            return;
        }
        M5.Log.printf("==== Can NOT increment\n");
        nfc_a.dump(block);
    }
    // Copy value block
    if (!nfc_a.mifareClassicRestoreValueBlock(block)) {
        M5_LOGE("Failed to restore %u", block);
        return;
    }
    if (!nfc_a.mifareClassicTransferValueBlock(block - 1)) {
        M5_LOGE("Failed to transfer %u", block);
        return;
    }
    M5.Log.printf("==== Copy from %u to %u\n", block, block - 1);
    nfc_a.dump(block);
    // Change read/write block and clear
    if (!nfc_a.mifareClassicWriteAccessCondition(block, READ_WRITE_BLOCK, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition%u", block);
        return;
    }
    uint8_t c[1]{};
    if (!nfc_a.write16(block, c, sizeof(c)) || !nfc_a.write16(block - 1, c, sizeof(c))) {
        M5_LOGE("Failed to Write %u/%u", block, block - 1);
        return;
    }
    M5.Log.printf("==== To be normal block\n");
    nfc_a.dump(block);
}
/**
* @brief Rechargeable e-wallet demonstration: create, decrement, and recharge value block
*
* Rechargeable e-wallet characteristics:
*   - Set amount on initialization
*   - Supports both decrement and recharge
*   - Sector trailer access: KeyB must be read-only
*   - Supports transfer to adjacent block
*
* Workflow:
*   1. Set sector trailer so KeyB is read-only
*   2. Authenticate sector with KeyB
*   3. Set block access to readable/writable
*   4. Initialize value block with initial amount
*   5. Change access condition to rechargeable mode
*   6. Demonstrate decrement operation
*   7. Demonstrate recharge (increment) operation
*   8. Demonstrate transfer operation
*   9. Restore access permissions to default
*   10. Restore block to normal
*
* @param block Block number to operate
* @param akey  MIFARE Classic KeyA (for authentication)
* @param bkey  MIFARE Classic KeyB (for authentication)
*/
void rechargeable_value_block(const uint8_t block, const Key& akey, const Key& bkey)
{
    auto& picc = nfc_a.activatedPICC();
    // Verify both blocks are user blocks
    if (!picc.isUserBlock(block) || !picc.isUserBlock(block - 1)) {
        M5_LOGE("block and block - 1 must be user block %u %u", block, block - 1);
        return;
    }
    // Auth A
    uint8_t stb = get_sector_trailer_block(block);
    if (!nfc_a.mifareClassicAuthenticateA(stb, akey)) {
        M5_LOGE("Failed to AUTH A %u/%u", block, stb);
        return;
    }
    // KeyB authentication is required for Increment operations
    // Additionally, KeyB must be read-only
    // Some cards may function even if the sector trailer access bit is 001, but strictly speaking, 110 or similar is
    // preferable
    // Change Sector trailer access bits
    //       RkeyA  WkeyA    RAb       WAb     ***RkeyB***   WkeyB
    // 011 | never | key B | key A|B | key B | ***never*** | key B |
    if (!nfc_a.mifareClassicWriteAccessCondition(stb, 0x03 /*011*/, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition %u", stb);
        return;
    }
    // Auth B
    if (!nfc_a.mifareClassicAuthenticateB(block, bkey)) {
        M5_LOGE("Failed to AUTH B %u/%u", block, stb);
        return;
    }
    // Change read/write block
    if (!nfc_a.mifareClassicWriteAccessCondition(block, READ_WRITE_BLOCK, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition %u", block);
        return;
    }
    // Write initial value
    if (!nfc_a.mifareClassicWriteValueBlock(block, 1234567)) {
        M5_LOGE("Failed to WriteValue %u", block);
        return;
    }
    // After writing the value, change it to the value block (rechargeable)
    if (!nfc_a.mifareClassicWriteAccessCondition(block, VALUE_BLOCK_RECHARGEABLE, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition %u", block);
        return;
    }
    M5.Log.printf("==== Initial value\n");
    nfc_a.dump(block);
    // Decrement and transfer value
    if (!nfc_a.mifareClassicDecrementValueBlock(block, 4567u)) {
        M5_LOGE("Failed to decrement %u", block);
        return;
    }
    M5.Log.printf("==== Decrement done\n");
    nfc_a.dump(block);
    // Increment and transfer value
    if (!nfc_a.mifareClassicIncrementValueBlock(block, 99u)) {
        M5_LOGE("Failed to increment %u", block);
        return;
    }
    M5.Log.printf("==== Increment done\n");
    nfc_a.dump(block);
    // Copy value block
    if (!nfc_a.mifareClassicRestoreValueBlock(block)) {
        M5_LOGE("Failed to restore %u", block);
        return;
    }
    if (!nfc_a.mifareClassicTransferValueBlock(block - 1)) {
        M5_LOGE("Failed to transfer %u", block);
        return;
    }
    M5.Log.printf("==== Copy from %u to %u\n", block, block - 1);
    nfc_a.dump(block);
    // Change read/write block and clear
    if (!nfc_a.mifareClassicWriteAccessCondition(block, READ_WRITE_BLOCK, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition%u", block);
        return;
    }
    // Clear both blocks
    uint8_t c[1]{};
    if (!nfc_a.write16(block, c, sizeof(c)) || !nfc_a.write16(block - 1, c, sizeof(c))) {
        M5_LOGE("Failed to Write %u/%u", block, block - 1);
        return;
    }
    // Restore access bits
    if (!nfc_a.mifareClassicWriteAccessCondition(stb, 0x01 /*001*/, akey, bkey)) {
        M5_LOGE("Failed to WriteAccessCondition %u", stb);
        return;
    }
    // Finally authenticate with KeyA once more
    if (!nfc_a.mifareClassicAuthenticateA(stb, akey)) {
        M5_LOGE("Failed to AUTH A %u/%u", block, stb);
        return;
    }
    M5.Log.printf("==== To be normal block\n");
    nfc_a.dump(block);
}
// Scan all sectors and restore any value blocks to normal read/write blocks
// Also restores sector trailer access bits to default (001)
// Tries multiple key combinations: KeyA/KeyB may have been changed by previous operations
void restore_all_value_blocks(const Key& akey, const Key& bkey)
{
    auto& picc = nfc_a.activatedPICC();
    uint8_t st_block{};
    uint32_t restored{};
    constexpr Key zero_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // Try to authenticate with multiple keys
    // Note: After a failed auth, PICC goes to HALT state. Need reactivate before retry.
    auto try_auth = [&](uint8_t stb) -> bool {
        if (nfc_a.mifareClassicAuthenticateA(stb, akey)) {
            M5_LOGI("Auth KeyA(default) OK for trailer %u", stb);
            return true;
        }
        nfc_a.reactivate();
        if (nfc_a.mifareClassicAuthenticateA(stb, zero_key)) {
            M5_LOGI("Auth KeyA(zero) OK for trailer %u", stb);
            return true;
        }
        nfc_a.reactivate();
        if (nfc_a.mifareClassicAuthenticateB(stb, bkey)) {
            M5_LOGI("Auth KeyB(default) OK for trailer %u", stb);
            return true;
        }
        nfc_a.reactivate();
        if (nfc_a.mifareClassicAuthenticateB(stb, zero_key)) {
            M5_LOGI("Auth KeyB(zero) OK for trailer %u", stb);
            return true;
        }
        nfc_a.reactivate();
        return false;
    };
    // Pass 1: Restore sector trailer access bits first
    // Some access conditions require KeyB for trailer writes
    for (uint_fast16_t stb = 3; stb < picc.blocks; stb = get_sector_trailer_block(stb + 1)) {
        if (!try_auth(stb)) {
            M5_LOGW("Cannot auth sector trailer %u, skip", stb);
            continue;
        }
        // Try with current auth (KeyA)
        if (nfc_a.mifareClassicWriteAccessCondition(stb, 0x01 /*001*/, akey, bkey)) {
            M5_LOGI("Restored trailer %u with KeyA", stb);
            continue;
        }
        M5_LOGW("KeyA write failed for trailer %u, trying KeyB...", stb);
        // KeyA write failed -> need KeyB auth for this trailer
        nfc_a.reactivate();
        if (nfc_a.mifareClassicAuthenticateB(stb, bkey)) {
            if (nfc_a.mifareClassicWriteAccessCondition(stb, 0x01, akey, bkey)) {
                M5_LOGI("Restored trailer %u with KeyB(default)", stb);
                continue;
            }
        }
        nfc_a.reactivate();
        if (nfc_a.mifareClassicAuthenticateB(stb, zero_key)) {
            if (nfc_a.mifareClassicWriteAccessCondition(stb, 0x01, akey, bkey)) {
                M5_LOGI("Restored trailer %u with KeyB(zero)", stb);
                continue;
            }
        }
        M5_LOGE("Cannot restore trailer %u", stb);
        nfc_a.reactivate();
    }
    // Pass 2: Find and restore value blocks
    st_block = 0;
    for (uint_fast16_t block = 0; block < picc.blocks; ++block) {
        uint8_t stb = get_sector_trailer_block(block);
        if (stb != st_block) {
            st_block = stb;
            if (!try_auth(stb)) {
                block = stb;
                continue;
            }
        }
        if (block == stb || !picc.isUserBlock(block)) {
            continue;
        }
        bool vb{};
        if (!nfc_a.mifareClassicIsValueBlock(vb, block)) {
            continue;
        }
        if (!vb) {
            continue;
        }
        M5.Log.printf("Found value block [%u], restoring...\n", block);
        // Change to read/write block
        if (!nfc_a.mifareClassicWriteAccessCondition(block, READ_WRITE_BLOCK, akey, bkey)) {
            M5_LOGE("Failed to change access condition %u", block);
            continue;
        }
        // Clear block data
        uint8_t c[1]{};
        if (!nfc_a.write16(block, c, sizeof(c))) {
            M5_LOGE("Failed to clear %u", block);
            continue;
        }
        ++restored;
    }
    M5.Log.printf("Restored %u value blocks\n", restored);
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
    lcd.printf("Put the tag and\ntouch/hold screen");
    M5.Log.printf("Put the tag and touch/hold screen\n");
}
void loop()
{
    M5StackChan.update();
    Units.update();// Update all registered Units
    bool clicked = M5.Touch.getDetail().wasClicked();  // For cross-block read/write test
    bool held    = M5.Touch.getDetail().wasHold();     // For single block read/write test
    if (clicked || held) {
        PICC picc{};
        if (nfc_a.detect(picc)) {// Detect a single card
            if (nfc_a.identify(picc) && nfc_a.reactivate(picc)) {// Identify card and reactivate for full parameters
                // Print card info: UID, type, user area size, total size
                M5.Log.printf("PICC:%s %s %u/%u\n", picc.uidAsString().c_str(), picc.typeAsString().c_str(),
                              picc.userAreaSize(), picc.totalSize());
                // Check if card is MIFARE Classic (supports e-wallet)
                if (picc.isMifareClassic()) {
                    if (clicked) {
                        lcd.fillScreen(TFT_BLUE);
                        M5.Log.print("Non rechargeable\n");
                        // Demonstrate non-rechargeable e-wallet: amount can only decrease, cannot recharge
                        non_rechargeable_value_block(picc.blocks - 2, keyA, keyB);
                        // nfc_a.dump(DEFAULT_KEY);
                    } else if (held) {
                        M5.Speaker.tone(4000, 30);
                        lcd.fillScreen(TFT_YELLOW);
                        M5.Log.print("Rechargeable\n");
                        // Demonstrate rechargeable e-wallet: amount can both decrease and recharge
                        rechargeable_value_block(picc.blocks - 2, keyA, keyB);
                        // restore_all_value_blocks(DEFAULT_KEY, DEFAULT_KEY);
                    }
                    M5.Log.printf("Please remove the PICC from the reader\n");
                } else {
                    M5.Log.printf("Not support the value block\n");
                }
                nfc_a.deactivate();
            } else {
                M5_LOGE("Failed to identify/activate %s", picc.uidAsString().c_str());
            }
        } else {
            M5.Log.printf("PICC NOT exists\n");
        }
        lcd.setCursor(0, 0);
        lcd.printf("Put the tag and\ntouch/hold screen");
        M5.Log.printf("Put the tag and touch/hold screen\n");
    }
}
