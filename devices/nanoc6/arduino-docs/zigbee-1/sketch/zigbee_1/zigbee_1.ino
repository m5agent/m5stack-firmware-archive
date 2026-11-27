#ifndef ZIGBEE_MODE_ZCZR
#error "Zigbee coordinator mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

// Define endpoint number
#define THERMOSTAT_ENDPOINT_NUMBER 5

// Create Zigbee Thermostat object (used to receive temperature data)
ZigbeeThermostat zbThermostat = ZigbeeThermostat(THERMOSTAT_ENDPOINT_NUMBER);

// Temperature data variables
float sensor_temp = 0.0;
float sensor_max_temp = 120.0;
float sensor_min_temp = -40.0;
float sensor_tolerance = 1.0;

// Temperature receive callback function
void receiveSensorTemp(float temperature) {
    Serial.printf("Temperature received: %.2f°C\n", temperature);
    sensor_temp = temperature;
}

// Sensor configuration receive callback function
void receiveSensorConfig(float min_temp, float max_temp, float tolerance) {
    Serial.printf("Sensor config: min=%.2f°C, max=%.2f°C, tolerance=%.2f°C\n",
                   min_temp, max_temp, tolerance);
    sensor_min_temp = min_temp;
    sensor_max_temp = max_temp;
    sensor_tolerance = tolerance;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== NanoC6 Zigbee Coordinator ===");

    // Set callback functions
    zbThermostat.onTempReceive(receiveSensorTemp);
    zbThermostat.onConfigReceive(receiveSensorConfig);

    // Configure device information
    zbThermostat.setManufacturerAndModel("Espressif", "ZigbeeThermostat");

    // Add endpoint
    Zigbee.addEndpoint(&zbThermostat);

    // Open network for 180 seconds
    Zigbee.setRebootOpenNetwork(180);

    // Start Coordinator
    Serial.println("Starting Zigbee Coordinator...");
    if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
        Serial.println("Zigbee failed to start!");
        Serial.println("Rebooting...");
        ESP.restart();
    }

    Serial.println("Zigbee Coordinator started");
    Serial.println("Network is open for 180 seconds");
    Serial.println("Waiting for temperature sensor to bind...");

    // Wait for End Device binding
    while (!zbThermostat.bound()) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nTemperature sensor bound successfully!");

    // Configure reporting interval only once
    zbThermostat.setTemperatureReporting(0, 10, 2);
    Serial.println("Temperature reporting configured");

    // Get sensor configuration
    zbThermostat.getSensorSettings();
}

void loop() {
    // Periodically print temperature data
    static uint32_t last_print = 0;
    if (millis() - last_print > 10000) {
        last_print = millis();

        int temp_percent = (int)((sensor_temp - sensor_min_temp) /
                                 (sensor_max_temp - sensor_min_temp) * 100);
        Serial.printf("Current temperature: %.2f°C (%d%%)\n",
                      sensor_temp, temp_percent);
    }

    delay(100);
}
