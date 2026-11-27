#ifndef ZIGBEE_MODE_ZCZR
#error "Zigbee coordinator mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

#define SWITCH_ENDPOINT_NUMBER 5

// ButtonA Pin (GPIO9)
#define BUTTON_A_PIN 9

// Zigbee Switch (To send On/Off command)
ZigbeeSwitch zbSwitch = ZigbeeSwitch(SWITCH_ENDPOINT_NUMBER);

// LED state
bool led_state = false;

// ButtonA debounce
unsigned long last_button_press = 0;
const unsigned long debounce_delay = 200;  // 200ms

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== NanoC6 Zigbee Coordinator (ButtonA Controller) ===");

    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    Serial.println("ButtonA (G9/GPIO9) initialized");

    zbSwitch.setManufacturerAndModel("Espressif", "ZigbeeSwitch");

    Zigbee.addEndpoint(&zbSwitch);

    // Network open for joining for 180 secs
    Zigbee.setRebootOpenNetwork(180);

    // Setup Coordinator
    Serial.println("Starting Zigbee Coordinator...");
    if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
        Serial.println("Zigbee failed to start!");
        Serial.println("Rebooting...");
        ESP.restart();
    }

    Serial.println("Zigbee Coordinator started");
    Serial.println("Network is open for 180 seconds");
    Serial.println("Waiting for LED device to bind...");

    while (!zbSwitch.bound()) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nLED device bound successfully!");
    Serial.println("\nPress ButtonA to toggle LED");
}

void loop() {
    // Check if ButtonA is pressed
    if (digitalRead(BUTTON_A_PIN) == LOW) {
        unsigned long current_time = millis();
        if (current_time - last_button_press > debounce_delay) {
            last_button_press = current_time;

            // Switch LED states
            led_state = !led_state;

            if (led_state) {
                Serial.println("Sending ON command...");
                zbSwitch.lightOn();
            } else {
                Serial.println("Sending OFF command...");
                zbSwitch.lightOff();
            }

            // Wait for ButtonA to release
            while (digitalRead(BUTTON_A_PIN) == LOW) {
                delay(10);
            }
        }
    }

    delay(10);
}
