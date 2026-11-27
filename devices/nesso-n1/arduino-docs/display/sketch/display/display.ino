#include <M5Unified.h>

void setup() {
  M5.begin();
  M5.Display.setRotation(1);  // Set to landscape mode

  // Set text properties
  M5.Display.setTextDatum(MC_DATUM);              // Middle-Center datum for text alignment
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);  // White text, black background
  M5.Display.setTextSize(2);

  // Clear the screen and draw the string
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.drawString("Hello, Nesso N1!", M5.Display.width() / 2, M5.Display.height() / 2);
}

void loop() {
  // Nothing to do in the loop
}
