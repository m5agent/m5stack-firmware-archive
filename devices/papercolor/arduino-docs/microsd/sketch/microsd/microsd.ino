#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <M5PM1.h>

static constexpr uint8_t SD_SPI_CS_PIN   = 47;
static constexpr uint8_t SD_SPI_SCK_PIN  = 15;
static constexpr uint8_t SD_SPI_MOSI_PIN = 13;
static constexpr uint8_t SD_SPI_MISO_PIN = 14;

static constexpr uint32_t IMAGE_SWITCH_MS = 60000;
static const char* const imagePaths[] = {
    "/picture_01.png",
    "/picture_02.png",
};

M5PM1 pm1;
size_t currentImageIndex = 0;
uint32_t lastSwitchMs    = 0;

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);

    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    M5.Display.setRotation(1);

    if (pm1.begin(&M5.In_I2C) != M5PM1_OK) {
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextColor(RED);
        M5.Display.drawString("PM1 init failed", M5.Display.width() / 2, M5.Display.height() / 2 + 20);
        Serial.println("PM1 init failed");
        for (;;) {
            delay(1000);
        }
    }

    pm1.setLdoEnable(true);
    pm1.pinMode(M5PM1_GPIO_NUM_0, OUTPUT);
    pm1.digitalWrite(M5PM1_GPIO_NUM_0, HIGH);  // PY_EPD_EN
    pm1.pinMode(M5PM1_GPIO_NUM_4, OUTPUT);
    pm1.digitalWrite(M5PM1_GPIO_NUM_4, HIGH);  // PY_SD_DET_EN
    pm1.pinMode(M5PM1_GPIO_NUM_3, OUTPUT);
    pm1.digitalWrite(M5PM1_GPIO_NUM_3, HIGH);  // PY_SD_PWR_EN
    pm1.pinMode(M5PM1_GPIO_NUM_1, INPUT_PULLUP);  // CARD_DEC

    if (pm1.digitalRead(M5PM1_GPIO_NUM_1) != LOW) {
        M5.Display.fillScreen(WHITE);
        M5.Display.setTextDatum(middle_center);
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextColor(RED);
        M5.Display.drawString("Card not inserted", M5.Display.width() / 2, M5.Display.height() / 2);
        Serial.println("SD card not inserted.");
        for (;;) {
            delay(1000);
        }
    }

    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        M5.Display.fillScreen(WHITE);
        M5.Display.setTextDatum(middle_center);
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextColor(RED);
        M5.Display.drawString("SD init failed", M5.Display.width() / 2, M5.Display.height() / 2);
        Serial.println("SD card initialization failed.");
        for (;;) {
            delay(1000);
        }
    }

    M5.Display.fillScreen(WHITE);
    if (!M5.Display.drawPngFile(SD, imagePaths[currentImageIndex])) {
        M5.Display.setTextDatum(middle_center);
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextColor(RED);
        M5.Display.drawString("PNG load failed", M5.Display.width() / 2, M5.Display.height() / 2);
        Serial.printf("Failed to draw PNG: %s\r\n", imagePaths[currentImageIndex]);
        for (;;) {
            delay(1000);
        }
    }

    Serial.printf("Displayed: %s\r\n", imagePaths[currentImageIndex]);
    lastSwitchMs = millis();
}

void loop()
{
    M5.update();

    const uint32_t now = millis();
    if (now - lastSwitchMs >= IMAGE_SWITCH_MS) {
        lastSwitchMs = now;
        currentImageIndex = (currentImageIndex + 1) % 2;
        M5.Display.fillScreen(WHITE);
        if (!M5.Display.drawPngFile(SD, imagePaths[currentImageIndex])) {
            M5.Display.fillScreen(WHITE);
            M5.Display.setTextDatum(middle_center);
            M5.Display.setFont(&fonts::Font4);
            M5.Display.setTextColor(RED);
            M5.Display.drawString("PNG load failed", M5.Display.width() / 2, M5.Display.height() / 2);
            Serial.printf("Failed to draw PNG: %s\r\n", imagePaths[currentImageIndex]);
        } else {
            Serial.printf("Displayed: %s\r\n", imagePaths[currentImageIndex]);
        }
    }

    delay(100);
}
