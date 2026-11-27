#include "M5Cardputer.h"

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
}

void loop() {
  M5Cardputer.Speaker.tone(7000, 100);  // frequency, duration
  delay(1000);
  M5Cardputer.Speaker.tone(4000, 20);  // frequency, duration
  delay(1000);
}
