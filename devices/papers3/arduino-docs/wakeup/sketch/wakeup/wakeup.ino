#include <M5Unified.h>
#include <M5GFX.h>

int myIndex = 0;

void setup() {
  M5.begin();
  M5.Display.setRotation(0);
  M5.Display.setFont(&fonts::FreeMonoBold24pt7b);

  M5.Display.clear();
  M5.Display.print("Sleep & Wakeup Test\n");
}

void loop() {
  myIndex++;

  M5.Display.setCursor(0, 100);
  M5.Display.printf("    %4d", myIndex);
  delay(250);

  if (myIndex % 20 == 0) {
    // M5.Power.timerSleep(5);              // in seconds
    // M5.Power.deepSleep(5000000, false);  // in microseconds
    M5.Power.lightSleep(5000000, false);    // in microseconds
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.print("Sleep & Wakeup Test\n");
  }
  if (myIndex >= 1000) {
    myIndex = 0;
  }
}
