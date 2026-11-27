#include <M5StickCPlus.h>

void setup() {
    M5.begin();
    M5.Beep.tone(4000, 300);
}

void loop() {
    M5.update();
}
