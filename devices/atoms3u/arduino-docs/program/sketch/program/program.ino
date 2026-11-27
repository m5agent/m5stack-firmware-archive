#include <M5Unified.h>

void setup() {
    M5.begin();
    Serial.begin(115200);    
}

void loop() {
    Serial.println("hello world!");
    delay(2000);
}
