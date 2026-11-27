#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedNFC.h>
#include <utility/M5IOE1_Class.hpp>
#include <vector>

using namespace m5::nfc;
using namespace m5::nfc::a;
using namespace m5::nfc::a::mifare;
using namespace m5::nfc::ndef;

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;      // Unit manager
m5::unit::UnitNFC unit{};         // NFC unit driver
m5::nfc::NFCLayerA nfc_a{unit};   // NFC-A protocol layer

constexpr uint8_t NFC_IRQ_PIN = 6;                             // NFC interrupt pin
constexpr auto NFC_CONTROL_PIN = m5::M5IOE1_Class::gpio4;      // NFC power-enable pin (IO expander)

// Layout constants
constexpr int STATUS_HEIGHT = 92;   // Bottom hint bar height
constexpr int TITLE_Y       = 30;   // Title position
constexpr int STATUS_Y      = 100;  // Status line below the title
constexpr int CONTENT_TOP   = 160;  // First data line position
constexpr int LINE_STEP     = 44;   // Vertical spacing between data lines

constexpr const char* KEY_HINT = "BtnA: Read    BtnB: Write";  // Fixed key hint text
}  // namespace

// Draw the fixed key-hint bar at the bottom
void drawBottomStatus(const char* message)
{
    const int top = lcd.height() - STATUS_HEIGHT;
    lcd.fillRect(0, top, lcd.width(), STATUS_HEIGHT, TFT_WHITE);
    lcd.drawFastHLine(0, top, lcd.width(), TFT_BLACK);  // Separator line
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    lcd.setTextDatum(middle_center);
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.drawString(message, lcd.width() / 2, top + STATUS_HEIGHT / 2);
}

// Render one page: title + status + data lines + bottom key hint
void renderPage(const char* status, const std::vector<String>& lines)
{
    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK, TFT_WHITE);

    // Title
    lcd.setTextDatum(top_center);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("PaperMono NFC Test", lcd.width() / 2, TITLE_Y);

    // Status / prompt line
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.drawString(status, lcd.width() / 2, STATUS_Y);

    // Data lines, stop before the bottom hint bar
    int y = CONTENT_TOP;
    for (auto&& line : lines) {
        if (y + LINE_STEP >= lcd.height() - STATUS_HEIGHT) break;
        lcd.drawString(line, lcd.width() / 2, y);
        y += LINE_STEP;
    }

    drawBottomStatus(KEY_HINT);
    lcd.display();
}

// Show a fatal error and halt
void stopOnError(const char* message)
{
    M5_LOGE("%s", message);
    renderPage(message, {});
    while (true) {
        delay(1000);
    }
}

// Initial screen
void drawReadyScreen()
{
    renderPage("Click BtnA to Read, BtnB to Write", {});
}

// Read NDEF and show the result under the title
void readNdef()
{
    std::vector<String> lines;

    // Verify the tag is NDEF formatted
    bool valid = false;
    if (!nfc_a.ndefIsValidFormat(valid)) {
        M5_LOGE("Failed to check NDEF format");
        renderPage("NDEF check failed", lines);
        return;
    }
    if (!valid) {
        M5.Log.printf("Data format is NOT NDEF\n");
        renderPage("Tag is not NDEF formatted", lines);
        return;
    }

    // Read raw NDEF data
    TLV message;
    if (!nfc_a.ndefRead(message)) {
        M5_LOGE("Failed to read NDEF");
        renderPage("NDEF read failed", lines);
        return;
    }

    // No message TLV present
    if (!message.isMessageTLV()) {
        M5.Log.printf("NDEF Message TLV is NOT exists\n");
        renderPage("No NDEF message", lines);
        return;
    }

    // Collect each record into display lines
    char buf[192];
    for (auto&& record : message.records()) {
        if (record.tnf() == TNF::Wellknown) {
            // Well-known type: show type and decoded payload
            const auto payload = record.payloadAsString();
            M5.Log.printf("SZ:%3u TNF:%u T:%s [%s]\n", record.payloadSize(), record.tnf(),
                          record.type(), payload.c_str());
            snprintf(buf, sizeof(buf), "Type: %s", record.type());
            lines.push_back(String(buf));
            snprintf(buf, sizeof(buf), "%s", payload.c_str());
            lines.push_back(String(buf));
        } else {
            // Other types: show type only
            M5.Log.printf("SZ:%3u TNF:%u T:%s\n", record.payloadSize(), record.tnf(),
                          record.type());
            snprintf(buf, sizeof(buf), "Type: %s", record.type());
            lines.push_back(String(buf));
        }
    }

    renderPage("NDEF Read Result", lines);
}

// Write NDEF, output written data into outLines for display
bool writeNdef(std::vector<String>& outLines)
{
    auto& picc = nfc_a.activatedPICC();

    // Format MIFARE Ultralight as NDEF if needed
    if (picc.isMifareUltralight() && !nfc_a.mifareUltralightChangeFormatToNDEF()) {
        M5_LOGE("Failed to format MIFARE Ultralight as NDEF");
        return false;
    }

    // Reserve one byte for the terminator TLV
    const uint32_t maxUserSize = picc.userAreaSize() > 1 ? picc.userAreaSize() - 1 : 0;
    if (maxUserSize == 0) {
        M5_LOGE("Invalid NDEF user area size");
        return false;
    }

    // Build the message with a URI and a text record
    TLV message{Tag::Message};
    Record uri;
    Record text;
    uri.setURIPayload("m5stack.com/", URIProtocol::HTTPS);
    text.setTextPayload("Hello PaperMono", "en");

    // Add URI record if it fits
    message.push_back(uri);
    if (message.required() > maxUserSize) {
        message.pop_back();
        return false;
    }
    outLines.push_back(String("URI: https://m5stack.com/"));

    // Add text record if it still fits
    message.push_back(text);
    if (message.required() > maxUserSize) {
        message.pop_back();
    } else {
        outLines.push_back(String("Text: Hello PaperMono"));
    }

    // Write to the tag
    if (message.records().empty() || !nfc_a.ndefWrite(message)) {
        M5_LOGE("Failed to write NDEF");
        outLines.clear();
        return false;
    }

    M5.Log.printf("Write NDEF OK!\n");
    return true;
}

void setup()
{
    Serial.begin(115200);

    // Keep current display content on boot
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    lcd.setRotation(0);
    lcd.setEpdMode(epd_mode_t::epd_text);

    // Enable NFC unit power via IO expander
    auto& ioe = M5.getIOExpander(0);
    ioe.setHighImpedance(NFC_CONTROL_PIN, false);
    ioe.setDirection(NFC_CONTROL_PIN, true);
    ioe.digitalWrite(NFC_CONTROL_PIN, true);
    delay(10);

    // Configure NFC to use the IRQ pin
    auto nfc_cfg = unit.config();
    nfc_cfg.using_irq = true;
    nfc_cfg.irq = NFC_IRQ_PIN;
    unit.config(nfc_cfg);

    // Register and start the NFC unit
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

    // Check button input
    const bool readRequested  = M5.BtnA.wasClicked();  // BtnA -> Read
    const bool writeRequested = M5.BtnB.wasClicked();  // BtnB -> Write
    if (!readRequested && !writeRequested) {
        delay(20);
        return;
    }

    // Prompt user to place the tag
    renderPage(readRequested ? "Place tag for READ..." : "Place tag for WRITE...", {});

    // Detect a tag in the field
    PICC picc{};
    if (!nfc_a.detect(picc)) {
        M5.Log.printf("PICC NOT exists\n");
        renderPage("PICC not found", {});
        return;
    }

    // Identify and (re)activate the tag
    if (!nfc_a.identify(picc) || !nfc_a.reactivate(picc)) {
        M5_LOGE("Failed to identify/activate %s", picc.uidAsString().c_str());
        renderPage("Identify failed", {});
        nfc_a.deactivate();
        return;
    }

    M5.Log.printf("PICC:%s %s %u/%u\n", picc.uidAsString().c_str(), picc.typeAsString().c_str(),
                  picc.userAreaSize(), picc.totalSize());

    // Dispatch based on tag capability and requested action
    if (!picc.supportsNDEF()) {
        M5.Log.printf("Not support the NDEF\n");
        renderPage("Tag does not support NDEF", {});
    } else if (readRequested) {
        readNdef();
    } else {
        std::vector<String> lines;
        const bool ok = writeNdef(lines);
        renderPage(ok ? "NDEF Write Result" : "NDEF write failed", lines);
    }

    M5.Log.printf("Please remove the PICC from the reader\n");
    nfc_a.deactivate();
}
