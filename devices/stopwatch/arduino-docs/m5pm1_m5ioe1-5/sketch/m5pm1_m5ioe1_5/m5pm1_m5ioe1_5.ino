#include <M5Unified.h>
#include <M5PM1.h>
#include "driver/rtc_io.h"

M5PM1 pm1;

void setup(void) {
  M5.begin();
  Serial.begin(115200);
  M5.Display.setFont(&fonts::FreeMonoBold12pt7b);
  M5.Display.clear();
  M5.Display.setCursor(40, 100);
  M5.Display.printf("RTC IRQ Test\n\n");
  M5.Display.println("     Press BtnA to Sleep");

  // Initialize PM1
  m5pm1_err_t err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);

  if (err == M5PM1_OK) {
    Serial.println("PM1 initialization successful");
    pm1.irqClearGpioAll();
    pm1.irqClearSysAll();
    pm1.irqClearBtnAll();

    pm1.irqSetGpioMaskAll(M5PM1_IRQ_MASK_ENABLE);
    pm1.irqSetSysMaskAll(M5PM1_IRQ_MASK_ENABLE);
    pm1.irqSetBtnMaskAll(M5PM1_IRQ_MASK_ENABLE);

    pm1.irqSetGpioMask(M5PM1_IRQ_GPIO0, M5PM1_IRQ_MASK_DISABLE);
    pm1.gpioSetMode(M5PM1_GPIO_NUM_0, M5PM1_GPIO_MODE_INPUT);
    pm1.gpioSetPull(M5PM1_GPIO_NUM_0, M5PM1_GPIO_PULL_UP);

    pm1.gpioSetMode(M5PM1_GPIO_NUM_1, M5PM1_GPIO_MODE_OUTPUT);
    pm1.gpioSetDrive(M5PM1_GPIO_NUM_1, M5PM1_GPIO_DRIVE_PUSHPULL);
    pm1.gpioSetFunc(M5PM1_GPIO_NUM_1, M5PM1_GPIO_FUNC_IRQ);
  } else {
    Serial.printf("PM1 initialization failed, error code: %d\n", err);
  }
  pm1.setSingleResetDisable(false);
  delay(200);
  M5.Rtc.clearIRQ();
  delay(200);
}

volatile bool pm1IrqTriggered = false;

void ARDUINO_ISR_ATTR pm1_irq_handler() {
    pm1IrqTriggered = true;
}

void loop(void) {
    M5.update();

    if (pm1IrqTriggered) {
        pm1IrqTriggered = false;
        M5.Rtc.setTimerIRQ(0);
        M5.Rtc.clearIRQ();
        pm1.irqClearGpioAll();
        pm1.irqClearSysAll();
        pm1.irqClearBtnAll();

        Serial.println("PM1 IRQ triggered");
        M5.Display.setCursor(40, 130);
        M5.Display.println("PM1 IRQ triggered");
    }

    if (M5.BtnA.wasPressed()) {
        if (M5.Rtc.setTimerIRQ(5000)){// Set Timer IRQ
            Serial.println("RTC IRQ enabled successfully");
            M5.Display.clear();
            M5.Display.setCursor(40, 100);
            M5.Display.printf("Wait for IRQ to wake up...");

            // Choose either of the two pieces of code below.
            pinMode(GPIO_NUM_12, INPUT_PULLUP);
            attachInterrupt(GPIO_NUM_12, pm1_irq_handler, FALLING);

            // esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);  // 0 = Low
            // rtc_gpio_pullup_en(GPIO_NUM_12);
            // Serial.println("Going to sleep now");
            // esp_deep_sleep_start();
        } else {
            Serial.println("Failed to enable RTC IRQ");
        }
    }
}
