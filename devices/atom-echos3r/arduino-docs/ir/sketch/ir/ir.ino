// Disable receiver functions will save program memory and RAM.
#define DISABLE_CODE_FOR_RECEIVER
#define NO_LED_SEND_FEEDBACK_CODE
#define NO_LED_FEEDBACK_CODE
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 47

#include "M5Unified.h"
// Third-party library: https://github.com/Arduino-IRremote/Arduino-IRremote
#include <IRremote.hpp>

uint16_t sAddress = 0x1234;
uint8_t sCommand  = 0x56;
uint8_t sRepeats  = 0;

void setup() {
  M5.begin();
  Serial.begin(115200);

  IrSender.begin(IR_TX_PIN);
}

void loop() {
  Serial.println("Sending standard NEC signal with 16 bit address: ");
  Serial.printf("address=0x%x, command=0x%x, repeats=%d\n\n", sAddress, sCommand, sRepeats);

  IrSender.sendNEC(sAddress, sCommand, sRepeats);

  sCommand += 1;
  delay(1000);
}
