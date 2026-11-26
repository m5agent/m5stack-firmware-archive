#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>

// ========== Correct CoreS3 SD card pin definitions ==========
#define SD_SPI_CS_PIN    4
#define SD_SPI_SCK_PIN  36
#define SD_SPI_MISO_PIN 35
#define SD_SPI_MOSI_PIN 37

void setup() {
  M5.begin();

  M5.Display.setFont(&fonts::FreeMono12pt7b);
  M5.Display.clear();

  // ========== SD card initialization (using correct pins) ==========
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    M5.Display.println("SD card not detected");
    while (1);
  } else {
    M5.Display.println("SD card detected");
  }
  delay(1000);

  // ========== Write test ==========
  M5.Display.println("SD card write test...");
  auto file = SD.open("/WriteTest.txt", FILE_WRITE, true);
  if (file) {
    file.print("Hello, world! \nSD card write success! \n");
    file.close();
    M5.Display.println("SD card write success");
  } else {
    M5.Display.println("Failed to create TXT file");
  }
  delay(1000);

  // ========== Read test ==========
  M5.Display.println("SD card read test...");

  if (SD.exists("/cores3_test_picture01.png")) {
    M5.Display.println("PNG file 01 detected");
  } else {
    M5.Display.println("PNG file 01 not detected");
  }

  if (SD.exists("/cores3_test_picture02.png")) {
    M5.Display.println("PNG file 02 detected");
  } else {
    M5.Display.println("PNG file 02 not detected");
  }
  delay(1000);
}

void loop() {
  M5.Display.drawPngFile(SD, "/cores3_test_picture01.png");
  delay(500);
  M5.Display.drawPngFile(SD, "/cores3_test_picture02.png");
  delay(500);
}
