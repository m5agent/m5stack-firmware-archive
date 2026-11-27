#include "M5Unified.h"

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
}

void loop()
{
    M5.update();
    if (M5.BtnA.wasPressed()) {
        Serial.println("BtnA Pressed");
    }
    if (M5.BtnB.wasPressed()) {
        Serial.println("BtnB Pressed");
    }
    if (M5.BtnC.wasPressed()) {
        Serial.println("BtnC Pressed");
    }
}
