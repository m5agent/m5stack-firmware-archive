#include <M5NanoC6.h>
#define BLUE_LED_PIN 7
bool blue_ledState = false;

void setup() {
  NanoC6.begin();
  pinMode(BLUE_LED_PIN, OUTPUT);
}

void loop() {
  blue_ledState = !blue_ledState;
  digitalWrite(BLUE_LED_PIN, blue_ledState);
  delay(1000);
}
