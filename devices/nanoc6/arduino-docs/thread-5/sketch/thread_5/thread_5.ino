#include "OThreadCLI.h"

void setup() {
  Serial.begin(115200);
  OThread.begin(false);  // No AutoStart - fresh start
  OThreadCLI.begin();
  Serial.println("OpenThread CLI started - type 'help' for a list of commands.");
  OThreadCLI.startConsole(Serial);
}

void loop() {}
