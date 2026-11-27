#include "M5Unified.h"
#include "test_audio.h"

// 8bit unsigned 44.1kHz mono (exclude wav header)
extern const uint8_t test_audio[46000];

void setup() {
  M5.begin();
  M5.Speaker.setVolume(64);

  M5.Speaker.tone(7000, 500);  // frequency, duration
  delay(500);
  M5.Speaker.tone(4000, 100);
  delay(500);
}

void loop() {
  M5.Speaker.playRaw(test_audio, sizeof(test_audio) / sizeof(test_audio[0]));
  while (M5.Speaker.isPlaying()) {
    delay(1);
  }
  delay(100);
}
