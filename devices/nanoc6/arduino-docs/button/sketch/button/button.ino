#include <M5NanoC6.h>

void setup() {
    NanoC6.begin();
}

void loop() {
    NanoC6.update();
    if (NanoC6.BtnA.wasPressed()) {
        Serial.println("Button A was pressed");
    }
    if (NanoC6.BtnA.wasHold()) {
        Serial.println("Button A was hold");
    }
    if (NanoC6.BtnA.wasReleased()) {
        Serial.println("Button A was released");
    }
}
