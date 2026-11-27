#include <M5Unified.h>
#include <M5PM1.h>
#include <M5IOE1.h>

M5PM1 pm1;
M5IOE1 ioe1;

#define IOE_MUX_EN   M5IOE1_PIN_1
#define IOE_AU_EN    M5IOE1_PIN_3
#define IOE_MT_PWM   M5IOE1_PIN_9
#define IOE_L3B_EN   M5IOE1_PIN_8
#define IOE_SPK_EN   M5IOE1_PIN_10
#define IOE_TP_EN    M5IOE1_PIN_4
#define IOE_OLED_RST M5IOE1_PIN_5

void setup(void)
{
    M5.begin();
    Serial.begin(115200);

    // Initialize PM1
    m5pm1_err_t pm1_err = pm1.begin(&M5.In_I2C, M5PM1_DEFAULT_ADDR, M5PM1_I2C_FREQ_100K);

    if (pm1_err == M5PM1_OK) {
        Serial.println("PM1 initialization successful");
    } else {
        Serial.printf("PM1 initialization failed, error code: %d\n", pm1_err);
    }
    pm1.setSingleResetDisable(false);

    // Initialize IOE1
    m5ioe1_err_t ioe1_err = ioe1.begin(&M5.In_I2C, M5IOE1_DEFAULT_ADDR, M5IOE1_I2C_FREQ_100K);

    if (ioe1_err == M5IOE1_OK) {
        Serial.println("IOE1 initialization successful");
    } else {
        Serial.printf("IOE1 initialization failed, error code: %d\n", ioe1_err);
    }
    ioe1.pinMode(IOE_MUX_EN, OUTPUT);
    ioe1.pinMode(IOE_AU_EN, OUTPUT);
    ioe1.pinMode(IOE_MT_PWM, OUTPUT);
    ioe1.pinMode(IOE_L3B_EN, OUTPUT);
    ioe1.pinMode(IOE_SPK_EN, OUTPUT);
    ioe1.pinMode(IOE_TP_EN, OUTPUT);
    ioe1.pinMode(IOE_OLED_RST, OUTPUT);
    
    ioe1.setPwmFrequency(2000);
    ioe1.setPwmDuty(M5IOE1_PWM_CH1, 0);   

    M5.Display.setFont(&fonts::FreeMonoBold12pt7b);
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("IOE1 Power Test Begin\n\n");
    delay(1000);
}

void loop(void)
{
    M5.update();
    // MUX IO
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("MUX IO set to UART0");
    ioe1.digitalWrite(IOE_MUX_EN, LOW);
    delay(1000);
    M5.Display.setCursor(40, 130);
    M5.Display.printf("MUX IO set to USB");
    ioe1.digitalWrite(IOE_MUX_EN, HIGH);
    delay(1000);

    // Speaker
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("Speaker off");
    ioe1.digitalWrite(IOE_SPK_EN, LOW);
    M5.Speaker.tone(10000, 100);
    M5.Display.setCursor(40, 130);
    M5.Display.printf("No sound");
    delay(1000);
    M5.Display.setCursor(40, 160);
    M5.Display.printf("1s later hear sound");
    ioe1.digitalWrite(IOE_SPK_EN, HIGH);
    delay(1000);
    M5.Speaker.tone(4000, 200);
    delay(1000);

    // Vibration Motor
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("Motor off");
    ioe1.setPwmDuty(M5IOE1_PWM_CH1, 0);
    M5.Display.setCursor(40, 130);
    M5.Display.printf("No vibration");
    delay(1000);
    M5.Display.setCursor(40, 160);
    M5.Display.printf("1s vibrate");
    ioe1.digitalWrite(IOE_MT_PWM, HIGH);
    ioe1.setPwmDuty(M5IOE1_PWM_CH1, 50);
    delay(1000);
    ioe1.setPwmDuty(M5IOE1_PWM_CH1, 0);
    delay(200);

    // Audio
    M5.Display.clear();
    M5.Display.setCursor(40, 100);
    M5.Display.printf("Audio off");
    ioe1.digitalWrite(IOE_AU_EN, LOW);
    M5.Speaker.end();
    M5.Speaker.begin();
    M5.Speaker.setVolume(128);
    M5.Speaker.tone(10000, 100);
    M5.Display.setCursor(40, 130);
    M5.Display.printf("No sound");
    delay(1000);
    M5.Display.setCursor(40, 160);
    M5.Display.printf("1s later hear sound");
    ioe1.digitalWrite(IOE_AU_EN, HIGH);
    M5.Speaker.end();
    M5.Speaker.begin();
    M5.Speaker.setVolume(128);
    delay(1000);
    M5.Speaker.tone(6000, 200);
    delay(1000);    
}
