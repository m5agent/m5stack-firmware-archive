// Happy birthday song

#include <M5Unified.h>
#include <M5GFX.h>

void setup() {
  M5.begin();
  M5.Display.setRotation(0);
  M5.Display.setFont(&fonts::FreeMonoBold24pt7b);

  M5.Display.print("\n\n   Happy birthday\n   to\n   you\n\n\n");
  M5.Display.print("      i  i  i\n     |:|:|:|:|\n    |||||||||||\n   {~~~~~~~~~~~}\n  [#############]\n  |#############|\n  |#############|\n  \\_____________/\n");

  M5.Speaker.setVolume(200);  // 0~255, bigger is louder
}

void loop() {
  M5.update();

  playHappyBirthday();
}

void playHappyBirthday() {
  int notes[] = {
    1320, 1320, 1480, 1320, 0, 1760, 1650, 0,
    1320, 1320, 1480, 1320, 0, 1980, 1760, 0,
    1320, 1320, 2600, 2200, 0, 1760, 1650, 1480, 0,
    2350, 2350, 2200, 1760, 0, 1980, 1760, 0
  };

  int durations[] = {
    300, 300, 600, 400, 200, 600, 600, 300,
    300, 300, 600, 400, 200, 600, 600, 300,
    300, 300, 600, 400, 200, 600, 600, 600, 300,
    300, 300, 600, 400, 200, 600, 600, 300
  };

  int count = sizeof(notes) / sizeof(int);
  for (int i = 0; i < count; i++) {
    if (notes[i] == 0) {
      delay(durations[i]);
    } else {
      M5.Speaker.tone(notes[i], durations[i]);
      delay(durations[i] + 30);
    }
  }
}
