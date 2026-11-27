#include <M5Unified.h>
#include <M5PM1.h>
#include <M5IOE1.h>

M5PM1 pm1;
M5IOE1 ioe1;

#define IOE_LORA_ANT_SW M5IOE1_PIN_2
#define IOE_LORA_RST    M5IOE1_PIN_10
#define IOE_NFC_EN      M5IOE1_PIN_4
#define IOE_LED_G       M5IOE1_PIN_8
#define IOE_LED_B       M5IOE1_PIN_9

void setup(void)
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);

    // Initialize PM1.
    const m5pm1_err_t pm1_err =
        pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);
    if (pm1_err == M5PM1_OK) {
        Serial.println("PM1 initialization successful");
    } else {
        Serial.printf("PM1 initialization failed, error code: %d\n", pm1_err);
    }
    pm1.setSingleResetDisable(false);

    // Enable the PaperMono LoRa power rail through PM1 GPIO2.
    pm1.gpioSetFunc(M5PM1_GPIO_NUM_2, M5PM1_GPIO_FUNC_GPIO);
    pm1.gpioSetMode(M5PM1_GPIO_NUM_2, M5PM1_GPIO_MODE_OUTPUT);
    pm1.gpioSetDrive(M5PM1_GPIO_NUM_2, M5PM1_GPIO_DRIVE_PUSHPULL);
    pm1.gpioSetOutput(M5PM1_GPIO_NUM_2, true);

    // Initialize IOE1.
    const m5ioe1_err_t ioe1_err =
        ioe1.begin(&M5.In_I2C, M5IOE1_DEFAULT_ADDR, M5IOE1_I2C_FREQ_100K);
    if (ioe1_err == M5IOE1_OK) {
        Serial.println("IOE1 initialization successful");
    } else {
        Serial.printf("IOE1 initialization failed, error code: %d\n", ioe1_err);
        while (true) delay(1000);
    }

    ioe1.pinMode(IOE_LORA_ANT_SW, OUTPUT);
    ioe1.pinMode(IOE_LORA_RST, OUTPUT);
    ioe1.pinMode(IOE_NFC_EN, OUTPUT);
    ioe1.pinMode(IOE_LED_G, OUTPUT);
    ioe1.pinMode(IOE_LED_B, OUTPUT);

    // Default states: LoRa receive path, LoRa released from reset, NFC on, LEDs off.
    ioe1.digitalWrite(IOE_LORA_ANT_SW, LOW);
    ioe1.digitalWrite(IOE_LORA_RST, HIGH);
    ioe1.digitalWrite(IOE_NFC_EN, HIGH);
    ioe1.digitalWrite(IOE_LED_G, LOW);
    ioe1.digitalWrite(IOE_LED_B, LOW);

    M5.Display.setEpdMode(epd_mode_t::epd_fast);
    M5.Display.setFont(&fonts::FreeMonoBold18pt7b);
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("IOE1 Power Test Begin\n\n");
    delay(1000);
}

void loop(void)
{
    M5.update();

    // Display frontlight.
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("Frontlight OFF");
    M5.Display.setBrightness(0);
    delay(1000);
    M5.Display.setBrightness(255);
    M5.Display.setCursor(40, 140);
    M5.Display.printf("Frontlight ON");
    delay(1000);

    // LoRa antenna switch.
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("LoRa antenna RX");
    ioe1.digitalWrite(IOE_LORA_ANT_SW, LOW);
    delay(1000);
    M5.Display.setCursor(40, 140);
    M5.Display.printf("LoRa antenna TX");
    ioe1.digitalWrite(IOE_LORA_ANT_SW, HIGH);
    delay(1000);

    // LoRa reset.
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("LoRa reset LOW");
    ioe1.digitalWrite(IOE_LORA_RST, LOW);
    delay(100);
    M5.Display.setCursor(40, 140);
    M5.Display.printf("LoRa reset HIGH");
    ioe1.digitalWrite(IOE_LORA_RST, HIGH);
    delay(1000);

    // NFC power enable.
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("NFC power OFF");
    ioe1.digitalWrite(IOE_NFC_EN, LOW);
    delay(1000);
    M5.Display.setCursor(40, 140);
    M5.Display.printf("NFC power ON");
    ioe1.digitalWrite(IOE_NFC_EN, HIGH);
    delay(1000);

    // RGB LED green and blue channels.
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("Green LED ON");
    ioe1.digitalWrite(IOE_LED_G, HIGH);
    ioe1.digitalWrite(IOE_LED_B, LOW);
    delay(1000);
    M5.Display.setCursor(40, 140);
    M5.Display.printf("Blue LED ON");
    ioe1.digitalWrite(IOE_LED_G, LOW);
    ioe1.digitalWrite(IOE_LED_B, HIGH);
    delay(1000);
    ioe1.digitalWrite(IOE_LED_G, LOW);
    ioe1.digitalWrite(IOE_LED_B, LOW);
    delay(200);
}
