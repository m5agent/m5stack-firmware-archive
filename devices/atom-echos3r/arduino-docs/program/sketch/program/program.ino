#include "M5Unified.h"

void setup() {
  M5.begin();
}

void loop() {
  M5.Speaker.tone(7000, 100);  // frequency, duration
  delay(1000);
  M5.Speaker.tone(4000, 20);  // frequency, duration
  delay(1000);
}
