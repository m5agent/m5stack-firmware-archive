#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

#define LIGHT_ENDPOINT_NUMBER 10

// Blue LED Pin (GPIO7)
#define BLUE_LED_PIN 7

// Zigbee controlled LED (listen for On/Off command)
ZigbeeLight zbLight = ZigbeeLight(LIGHT_ENDPOINT_NUMBER);

// LED toggle
void setLED(bool state) {
    digitalWrite(BLUE_LED_PIN, state ? HIGH : LOW); // NanoC6 Blue LED is active LOW
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== NanoC6 Zigbee End Device (LED) ===");

    // Set Blue LED Pin
    pinMode(BLUE_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, HIGH);
    Serial.println("Blue LED (G4/GPIO4) initialized");

    zbLight.setManufacturerAndModel("Espressif", "ZigbeeLight");

    zbLight.onLightChange(setLED);

    Zigbee.addEndpoint(&zbLight);

    // Setup End Device
    Serial.println("Starting Zigbee End Device...");
    if (!Zigbee.begin()) {
        Serial.println("Zigbee failed to start!");
        Serial.println("Rebooting in 5 seconds...");
        delay(5000);
        ESP.restart();
    }

    Serial.println("Zigbee End Device started");
    Serial.println("Searching for Zigbee network...");

    // Wait for connecting to network
    while (!Zigbee.connected()) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nConnected to Zigbee network!");
    Serial.println("Waiting for commands from Coordinator...");
}

void loop() {
    // Zigbee network will auto serial print
    delay(100);
}
