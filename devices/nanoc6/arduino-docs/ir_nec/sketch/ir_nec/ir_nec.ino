#include <IRremote.hpp>
#define IR_TX_PIN 3  // IR transmitter pin for NanoC6

uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

void setup() {
  Serial.begin(115200);

  // Initialize IR transmitter
  IrSender.begin(IR_TX_PIN, DISABLE_LED_FEEDBACK);

  Serial.println("NanoC6 IR NEC Transmitter");
  Serial.println("Sending incremental NEC codes...");
}

void loop() {
  Serial.println();
  Serial.print("Send now: address=0x1111, command=0x");
  Serial.print(sCommand, HEX);
  Serial.print(", repeats=");
  Serial.print(sRepeats);
  Serial.println();

  // Display on Serial
  Serial.println("--- IR NEC SEND ---");
  Serial.println("ADDR: 0x1111");
  Serial.print("CMD: 0x");
  Serial.println(sCommand, HEX);
  Serial.println("-------------------");

  // Send NEC IR signal
  Serial.println("Sending standard NEC with 16 bit address");

  IrSender.sendNEC(0x1111, sCommand, sRepeats);

  // Increment command for next transmission
  sCommand += 1;

  delay(1000);  // Wait 1 second between transmissions
}
