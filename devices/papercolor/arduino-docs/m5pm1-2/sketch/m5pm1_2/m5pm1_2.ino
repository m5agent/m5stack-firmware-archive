#include <M5Unified.h>
#include <M5PM1.h>

M5Canvas canvas(&M5.Display);
M5PM1 pm1;

void setup(void)
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);

    // Initialize PM1
    m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);

    if (err == M5PM1_OK) {
        Serial.println("PM1 initialization successful");
        pm1.gpioSetWakeEnable(M5PM1_GPIO_NUM_2, true);
        pm1.gpioSetWakeEdge(M5PM1_GPIO_NUM_2, M5PM1_GPIO_WAKE_FALLING);  // Falling edge
    } else {
        Serial.printf("PM1 initialization failed, error code: %d\n", err);
    }

    M5.Display.setEpdMode(epd_mode_t::epd_fastest);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    canvas.setFont(&fonts::FreeMonoBold12pt7b);
    canvas.clear();
    canvas.setCursor(40, 100);
    canvas.printf("RTC Wakeup Test\n\n");
    canvas.println("   Press BtnA to Sleep");
    canvas.pushSprite(0, 0);
}

void loop(void)
{
    M5.update();

    if (M5.BtnA.wasPressed()) {
        M5.Rtc.clearIRQ();

        pm1.setLdoEnable(true);
        pm1.ldoSetPowerHold(true);
        pm1.setLedEnLevel(true);

        canvas.clear();
        canvas.setCursor(40, 100);
        canvas.printf("Power OFF");
        canvas.setCursor(40, 130);
        canvas.printf("5s later wakeup");
        canvas.pushSprite(0, 0);

        if (M5.Rtc.setTimerIRQ(5000)) {  // 5s later wakeup
            Serial.println("RTC IRQ enabled successfully");
            delay(100);
            pm1.shutdown();
        } else {
            Serial.println("Failed to enable RTC IRQ");
        }
    }
}
