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
        pm1.gpioSetWakeEnable(M5PM1_GPIO_NUM_0, true);
        pm1.gpioSetWakeEdge(M5PM1_GPIO_NUM_0, M5PM1_GPIO_WAKE_FALLING);  // Falling edge
    } else {
        Serial.printf("PM1 initialization failed, error code: %d\n", err);
    }
    pm1.setSingleResetDisable(false);

    M5.Display.setFont(&fonts::FreeMonoBold12pt7b);
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("RTC Wakeup Test\n\n");
    M5.Display.println("     Press BtnA to Sleep");
}

void loop(void)
{
    M5.update();
    if (M5.BtnA.wasPressed()) {
        M5.Rtc.clearIRQ();

        if (M5.Rtc.setTimerIRQ(5000)){// 5s later wakeup
            Serial.println("RTC IRQ enabled successfully");
            M5.Display.clear();
            M5.Display.setCursor(40, 100);
            M5.Display.printf("Power OFF");
            M5.Display.setCursor(40, 130);
            M5.Display.printf("5s later wakeup");
            delay(500);
            // Shutdown
            pm1.setLdoEnable(true);
            pm1.ldoSetPowerHold(true);
            pm1.setLedEnLevel(true);
            pm1.shutdown();
        } else {
            Serial.println("Failed to enable RTC IRQ");
        }
    }
}
