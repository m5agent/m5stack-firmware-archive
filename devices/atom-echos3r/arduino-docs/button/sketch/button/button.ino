#include "M5Unified.h"

void setup() {
  M5.begin();
  Serial.begin(115200);
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    Serial.println("BtnA was pressed");
  }
  if (M5.BtnA.wasReleased()) {
    Serial.println("BtnA was released");
  }
  if (M5.BtnA.wasSingleClicked()) {
    Serial.println("BtnA was single clicked");
  }
  if (M5.BtnA.wasDoubleClicked()) {
    Serial.println("BtnA was double clicked");
  }
  if (M5.BtnA.wasHold()) {
    Serial.println("BtnA was held");
  }
  if (M5.BtnA.wasReleaseFor(5000)) {  // ms
    Serial.println("BtnA was released after being held for 5000 ms");
  }

  delay(5);
}
