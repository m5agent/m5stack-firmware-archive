#include "M5Unified.h"
#include <IRremote.hpp>

#define IR_SEND_PIN    47    

// Demo parameters for NEC protocol
uint16_t address = 0x0000;     // Starting device address
uint8_t  command = 0x55;       // Starting command value
uint8_t  repeats = 0;          // Number of repeat transmissions

void setup() {
    M5.begin();                // Initialize M5Stack device
    Serial.begin(115200);      // Start serial communication at 115200 baud
    Serial.println("AtomS3U IRremote example");
    delay(200);                // Wait for serial port to stabilize

    pinMode(IR_SEND_PIN, OUTPUT);// Essential! Otherwise can not transmit

    // Initialize IR communication
    IrSender.begin(DISABLE_LED_FEEDBACK);  // Initialize IR sender without LED feedback
    IrSender.setSendPin(IR_SEND_PIN);      // Assign transmitter pin

    Serial.printf("IR Send Pin: %d\n", IR_SEND_PIN);
    delay(500); // Wait for hardware components to stabilize
}

void loop() {
    // Send infrared signal using NEC protocol
    Serial.printf("Send NEC: addr=0x%04x, cmd=0x%02x\n", address, command);
    IrSender.sendNEC(address, command, repeats);

    // Update transmission parameters for next cycle
    address += 0x0001;  // Increment device address
    command += 0x01;     // Increment command code
    repeats  = 0;        // Disable repeat frames (set >0 to test repeats)

    delay(2000);
}
