#include <M5Unified.h>
#include <Unit_Sonic.h>

SONIC_I2C sensor;

void setup()
{
    M5.begin();
    // sensor.begin(TwoWire* wire, uint8_t addr, uint8_t sda, uint8_t scl, uint32_t speed);
    sensor.begin(&Wire, 0x57, 2, 1, 400000U);
    M5.Display.setColorDepth(1);
    M5.Display.setFont(&fonts::Orbitron_Light_32);
    M5.Display.setTextDatum(middle_center);
}

int point      = 0;
int last_point = 0;

void loop()
{
    float Distance = sensor.getDistance();
    Serial.printf("Distance: %.2fmm\r\n", Distance);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.drawString(String(Distance) + "mm", M5.Display.width() / 2, M5.Display.height() / 2);
    delay(100);
}
