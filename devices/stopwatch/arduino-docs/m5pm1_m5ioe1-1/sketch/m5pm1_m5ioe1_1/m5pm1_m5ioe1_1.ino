#include <M5Unified.h>
#include <M5PM1.h>

M5PM1 pm1;

void setup(void)
{
    M5.begin();
    Serial.begin(115200);

    // Initialize PM1
    m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);

    if (err == M5PM1_OK) {
        Serial.println("PM1 initialization successful");
    } else {
        Serial.printf("PM1 initialization failed, error code: %d\n", err);
    }
    pm1.setSingleResetDisable(false);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextFont(&fonts::FreeMonoBold12pt7b);
    M5.Display.setCursor(80, 150);
    M5.Display.println("Timer Power Test");
    M5.Display.println("    BtnA: After 10s ON");
    M5.Display.println("    BtnB: After 10s OFF");
}

void loop(void)
{
    M5.update();
    if (M5.BtnA.wasPressed()) {
        M5.Display.fillScreen(BLACK);
        M5.Display.setCursor(40, 80);
        M5.Display.println("Shutdown");
        M5.Display.println("  After 10s");
        M5.Display.println("     Power ON");
        delay(500);
        pm1.timerSet(10, M5PM1_TIM_ACTION_POWERON);
        pm1.shutdown();
    }
    if (M5.BtnB.wasPressed()) {
        M5.Display.fillScreen(BLACK);
        M5.Display.setCursor(40, 80);
        M5.Display.println("  After 10s");
        M5.Display.println("     Power OFF");
        delay(500);
        pm1.timerSet(10, M5PM1_TIM_ACTION_POWEROFF);
    }
} 
