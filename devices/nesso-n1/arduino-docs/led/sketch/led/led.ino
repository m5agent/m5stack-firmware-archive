#include "M5Unified.h"

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
}

void loop() {

  // LED_BUILTIN at E1.P7
  auto& ioe = M5.getIOExpander(1);
  ioe.digitalWrite(7, false);
  delay(1000);
  ioe.digitalWrite(7, true);
  delay(1000);
}
