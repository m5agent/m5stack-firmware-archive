#include <M5StackChan.h>

void setup() {
    M5StackChan.begin();
    M5StackChan.Display().setFont(&fonts::FreeMonoBold12pt7b);

    M5StackChan.Display().clear();
    M5StackChan.Display().setCursor(0, 0);
    M5StackChan.Display().print("StackChan Battery");
}

void loop() {
    M5StackChan.update();
    float voltage = M5StackChan.getBatteryVoltage();
    float current = M5StackChan.getBatteryCurrent() * 1000;

    M5StackChan.Display().setCursor(0, 50);
    M5StackChan.Display().printf("Voltage: %0.2fV", voltage);

    M5StackChan.Display().setCursor(0, 100);
    M5StackChan.Display().printf("Current: %0.2fmA", current);

    delay(1000);
}
