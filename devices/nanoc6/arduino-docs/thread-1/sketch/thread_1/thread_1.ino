#include "OThreadCLI.h"
#include "OThreadCLI_Util.h"

// The first device to start Thread will be the Leader
// Next devices will be Router or Child

void setup() {
  Serial.begin(115200);
  OThread.begin();  // AutoStart using Thread default settings
  OThreadCLI.begin();
  OThread.otPrintNetworkInformation(Serial);  // Print Current Thread Network Information
}

void loop() {
  Serial.print("Thread Node State: ");
  Serial.println(OThread.otGetStringDeviceRole());
  delay(5000);
}
