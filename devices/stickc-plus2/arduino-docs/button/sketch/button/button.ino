#include "M5StickCPlus2.h"

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);
}

void loop() {
    StickCP2.update();
    if (StickCP2.BtnA.wasPressed()) {
        Serial.println("A Btn Pressed");
    }
    if (StickCP2.BtnA.wasReleased()) {

        Serial.println("A Btn Released");
    }
    if (StickCP2.BtnB.wasPressed()) {
        Serial.println("B Btn Pressed");
    }
    if (StickCP2.BtnB.wasReleased()) {
        Serial.println("B Btn Released");
    }
    if (StickCP2.BtnPWR.wasClicked()) {
        Serial.println("PWR Btn Clicked");
    }
    if (StickCP2.BtnPWR.wasHold()) {
        Serial.println("PWR Btn Hold");
    }
}
