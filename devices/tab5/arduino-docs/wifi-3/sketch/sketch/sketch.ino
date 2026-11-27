#include "M5Unified.h"

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    // Use internal Wi-Fi antenna
    auto& ioe = M5.getIOExpander(0);
    ioe.digitalWrite(0, false);
}

void loop()
{
}
