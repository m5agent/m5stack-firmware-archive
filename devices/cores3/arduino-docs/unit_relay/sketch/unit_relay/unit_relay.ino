#include <M5Unified.h>

#define RELAY_PIN 9

bool relay_no_status = false;

void setup()
{
    M5.begin();
    M5.Display.setFont(&fonts::FreeMonoBold12pt7b);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    M5.Display.setCursor(10, 40);
    M5.Display.println("Relay NO & COM Open");
}

void loop(void)
{
    M5.update();
    auto t = M5.Touch.getDetail();

    if (t.wasClicked() || M5.BtnA.wasClicked()) {
        relay_no_status = !relay_no_status;
        if (relay_no_status) {
            M5.Display.clear();
            M5.Display.setCursor(10, 40);
            M5.Display.println("Relay NO & COM Closed");
            digitalWrite(RELAY_PIN, HIGH);
        } else {
            M5.Display.clear();
            M5.Display.setCursor(10, 40);
            M5.Display.println("Relay NO & COM Open");
            digitalWrite(RELAY_PIN, LOW);
        }
    }
}
