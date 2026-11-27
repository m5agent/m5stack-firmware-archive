#include <M5Unified.h>
#include <M5PM1.h>

M5PM1 pm1;

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);

    const m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);
    if (err != M5PM1_OK) {
        Serial.printf("M5PM1 init failed: %d\n", err);
        while (true) {
            delay(1000);
        }
    }

    M5.Display.setEpdMode(epd_mode_t::epd_fast);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::FreeSansBold18pt7b);
    M5.Display.drawString("BtnA: power on after 10s", M5.Display.width() / 2, 280);
    M5.Display.drawString("BtnB: power off after 10s", M5.Display.width() / 2, 340);
}

void loop(void)
{
    M5.update();
    if (M5.BtnA.wasPressed()) {
        M5.Display.clear();
        M5.Display.setCursor(40, 80);
        M5.Display.println("Shutdown");
        M5.Display.println("  After 10s");
        M5.Display.println("     Power ON");
        delay(500);
        pm1.timerSet(10, M5PM1_TIM_ACTION_POWERON);
        pm1.shutdown();
    }
    if (M5.BtnB.wasPressed()) {
        M5.Display.clear();
        M5.Display.setCursor(40, 80);
        M5.Display.println("  After 10s");
        M5.Display.println("     Power OFF");
        delay(500);
        pm1.timerSet(10, M5PM1_TIM_ACTION_POWEROFF);
    }
}
