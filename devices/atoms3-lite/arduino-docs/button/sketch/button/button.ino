#include "M5AtomS3.h"

void setup() {
    AtomS3.begin();
}

void loop() {
    AtomS3.update();
    if (AtomS3.BtnA.wasPressed()) {

        Serial.println("Pressed");
    }
    if (AtomS3.BtnA.wasReleased()) {

        Serial.println("Released");
    }
}
