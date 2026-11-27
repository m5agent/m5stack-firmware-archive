#include "M5Dial.h"

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, false, false);  // encoder, RFID
  Serial.begin(115200);
}

void loop() {
  M5Dial.update();

  if (M5Dial.BtnA.wasPressed()) {
    Serial.println("Button A Pressed");
  }
  if (M5Dial.BtnA.wasReleased()) {
    Serial.println("Button A Released");
  }
}
