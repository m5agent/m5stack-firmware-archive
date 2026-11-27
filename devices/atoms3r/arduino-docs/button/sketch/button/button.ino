#include "M5Unified.h"

void setup() {
    M5.begin();
    Serial.begin(115200);
}

void loop() {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        Serial.println("Pressed");
    }
    if (M5.BtnA.wasReleased()) {
        Serial.println("Released");
    }
}
