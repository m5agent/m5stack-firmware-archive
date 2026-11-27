#include <M5Unified.h>
#include <M5GFX.h>

void setup() {
  M5.begin();
  M5.Display.setRotation(0);
  M5.Display.setFont(&fonts::DejaVu40);

  M5.Display.print("\nStart playing\nmusical scale");

  M5.Speaker.setVolume(200);  // 0~255, bigger is louder
}

void loop() {
  M5.update();

  M5.Speaker.tone(880, 500);  // frequency (Hz), duration (ms)
  delay(500);
  M5.Speaker.tone(990, 500);
  delay(500);
  M5.Speaker.tone(1120, 500);
  delay(500);
  M5.Speaker.tone(1180, 500);
  delay(500);
  M5.Speaker.tone(1320, 500);
  delay(500);
  M5.Speaker.tone(1480, 500);
  delay(500);
  M5.Speaker.tone(1650, 500);
  delay(500);
  M5.Speaker.tone(1760, 500);
  delay(1000);

  M5.Speaker.tone(1760, 500);
  delay(500);
  M5.Speaker.tone(1650, 500);
  delay(500);
  M5.Speaker.tone(1480, 500);
  delay(500);
  M5.Speaker.tone(1320, 500);
  delay(500);
  M5.Speaker.tone(1180, 500);
  delay(500);
  M5.Speaker.tone(1120, 500);
  delay(500);
  M5.Speaker.tone(990, 500);
  delay(500);
  M5.Speaker.tone(880, 500);
  delay(1000);
}
