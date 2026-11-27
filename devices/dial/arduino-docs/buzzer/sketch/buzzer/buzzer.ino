#include "M5Dial.h"

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, false, false);  // encoder, RFID
}

void loop() {
  M5Dial.Speaker.tone(10000, 100);  // frequency, duration
  delay(1000);
  M5Dial.Speaker.tone(4000, 20);  // frequency, duration
  delay(1000);
}
