#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"
#include <Wire.h>

// Define endpoint number
#define TEMP_SENSOR_ENDPOINT_NUMBER 10

// Define SHT30 I2C address
#define SHT30_I2C_ADDR 0x44

// I2C pins of NanoH2 Grove interface
#define I2C_SDA 2   // G2 = GPIO2 (Yellow wire)
#define I2C_SCL 1   // G1 = GPIO1 (White wire)

// Create Zigbee temperature sensor object
ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);

// Temperature data variable
float temperature = 0.0;

// Task synchronization flag
volatile bool temp_updated = false;

/************************ SHT30 Temperature Read Function (Temperature Only) *****************************/
bool readSHT30Temperature(float &temp) {
    // Send measurement command (high repeatability measurement)
    Wire.beginTransmission(SHT30_I2C_ADDR);
    Wire.write(0x2C);  // Measurement command high byte
    Wire.write(0x06);  // Measurement command low byte

    if (Wire.endTransmission() != 0) {
        Serial.println("SHT30 communication failed!");
        return false;
    }

    // Wait for measurement to complete (SHT30 requires about 15ms)
    delay(20);

    // Request to read 6 bytes (temperature 3 bytes + humidity 3 bytes)
    Wire.requestFrom(SHT30_I2C_ADDR, 6);

    if (Wire.available() < 6) {
        Serial.println("SHT30 data not available!");
        return false;
    }

    // Read temperature data (first 3 bytes)
    uint8_t tempData[3];
    for (int i = 0; i < 3; i++) {
        tempData[i] = Wire.read();
    }

    // Skip humidity data (last 3 bytes)
    for (int i = 0; i < 3; i++) {
        Wire.read();
    }

    // Calculate temperature (formula: -45 + 175 * (rawTemp / 65535))
    uint16_t rawTemp = (tempData[0] << 8) | tempData[1];
    temp = -45.0 + 175.0 * ((float)rawTemp / 65535.0);

    // Data validity check
    if (temp < -40.0 || temp > 125.0) {
        Serial.printf("Invalid temperature: %.2f°C\n", temp);
        return false;
    }

    return true;
}

/************************ Temperature Read Task *****************************/
static void temp_sensor_value_update(void *arg) {
    for (;;) {
        // Read SHT30 temperature
        if (readSHT30Temperature(temperature)) {
            Serial.printf("Temperature: %.2f°C\n", temperature);

            // Update temperature value to Zigbee endpoint
            zbTempSensor.setTemperature(temperature);

            // Set flag to notify main loop for reporting
            temp_updated = true;
        } else {
            Serial.println("Failed to read SHT30");
        }

        delay(10000);  // Read every 10 seconds
    }
}

/********************* Arduino Main Functions ***************************/
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== NanoC6 End Device + Unit ENV III (Temperature Only) ===");

    // Initialize I2C (using correct pins)
    Wire.begin(I2C_SDA, I2C_SCL, 100000);  // 100kHz
    Serial.printf("I2C initialized (SDA=GPIO%d, SCL=GPIO%d, 100kHz)\n",
                  I2C_SDA, I2C_SCL);

    // Add delay to wait for bus stabilization
    delay(100);

    // Test SHT30 connection
    Serial.println("Scanning I2C bus...");
    Wire.beginTransmission(SHT30_I2C_ADDR);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
        Serial.println("SHT30 detected at address 0x44");
    } else {
        Serial.printf("SHT30 not found! Error code: %d\n", error);
        Serial.println("\n   Expected wiring:");
        Serial.println("   - Black (GND) → GND");
        Serial.println("   - Red (5V) → 5V");
        Serial.println("   - Yellow (SDA) → G2 (GPIO2)");
        Serial.println("   - White (SCL) → G1 (GPIO1)");
        while (1) delay(1000);  // Stop execution
    }

    // Configure Zigbee temperature sensor
    zbTempSensor.setManufacturerAndModel("Espressif", "TempSensor");
    zbTempSensor.setMinMaxValue(-40, 120);  // Set temperature range
    zbTempSensor.setTolerance(1);  // Set tolerance ±1°C

    // Add endpoint to Zigbee core
    Zigbee.addEndpoint(&zbTempSensor);

    // Start Zigbee End Device
    Serial.println("Starting Zigbee End Device...");
    if (!Zigbee.begin()) {
        Serial.println("Zigbee failed to start!");
        Serial.println("Rebooting in 5 seconds...");
        delay(5000);
        ESP.restart();
    }

    Serial.println("Zigbee End Device started");
    Serial.println("Connecting to network...");

    // Wait for Zigbee network connection
    while (!Zigbee.connected()) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nConnected to Zigbee network!");

    // Create temperature read task (stack size 4096 bytes)
    xTaskCreate(temp_sensor_value_update, "temp_sensor_update", 4096, NULL, 10, NULL);

    // Configure reporting interval
    zbTempSensor.setReporting(1, 0, 1);
    Serial.println("Temperature reporting configured");
}

void loop() {
    // Report only when temperature is updated
    if (temp_updated) {
        temp_updated = false;
        zbTempSensor.reportTemperature();
        Serial.println("Temperature reported to Coordinator");
    }

    delay(100);
}
