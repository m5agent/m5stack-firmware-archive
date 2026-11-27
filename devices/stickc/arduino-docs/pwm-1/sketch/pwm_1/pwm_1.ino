#include <M5StickC.h>

void setup() {
  M5.begin();
  ledcSetup(8, 1,10);  
          // Set the frequency of LEDC channel 8 to 1 and the resolution to
          // 10 bits, that is, the duty cycle can be selected from 0 to
          // 1023. Set LEDC channel 8 frequency to 1, resolution to 10 bits, meaning the duty cycle can range from 0 to 1023.
  ledcAttachPin(32,8);
          // Set LEDC channel 8 to output on IO32.  Set LEDC channel 8 for output on IO32.
}

void loop() {}
