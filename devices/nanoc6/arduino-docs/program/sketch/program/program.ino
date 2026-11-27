#include <Adafruit_NeoPixel.h>

#define LED_PIN    20
#define ENABLE_PIN 19
#define NUM_LEDS   1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);
  strip.begin();
  strip.show();
}

void loop() {
    //RED
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    delay(100);

    //GREEN
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();
    delay(100);

    //BLUE
    strip.setPixelColor(0, strip.Color(0, 0, 255));
    strip.show();
    delay(100);
}
