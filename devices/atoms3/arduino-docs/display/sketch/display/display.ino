#include "M5AtomS3.h"

void setup() {
    AtomS3.begin();
    AtomS3.Display.setTextDatum(middle_center);
    AtomS3.Display.fillScreen(WHITE);
    AtomS3.Display.fillCircle(AtomS3.Display.width() / 2,
                              AtomS3.Display.height() / 2, 30, ORANGE);
    AtomS3.Display.drawString("Hello", AtomS3.Display.width() / 2,
                              AtomS3.Display.height() / 2);
}

void loop() {
}
