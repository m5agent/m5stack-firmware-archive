#include <M5StickC.h>
int freq = 1800;
int channel = 0;
int resolution_bits = 8;
int buzzer = 2;

void setup() {
  M5.begin();
  ledcSetup(channel, freq, resolution_bits);
  ledcAttachPin(buzzer, channel);
}
void loop() {
    ledcWrite(channel, 128);
    delay(1000);
    ledcWrite(channel, 0);
    delay(1000);
}
