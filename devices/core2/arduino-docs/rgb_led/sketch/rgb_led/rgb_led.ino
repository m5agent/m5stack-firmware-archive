#include <M5Unified.h>
#include "utility/led/LED_Strip_Class.hpp"

constexpr int LED_DATA_PIN = 25;
constexpr size_t LED_COUNT = 10;

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);

    auto busled = std::make_shared<m5::LedBus_RMT>();
    auto buscfg = busled->getConfig();
    buscfg.pin_data = LED_DATA_PIN;
    busled->setConfig(buscfg);

    auto led_strip = std::make_shared<m5::LED_Strip_Class>();
    auto ledcfg = led_strip->getConfig();
    ledcfg.led_count = LED_COUNT;
    ledcfg.byte_per_led = 3;
    ledcfg.color_order = m5::LED_Strip_Class::config_t::color_order_grb;
    led_strip->setBus(busled);
    led_strip->setConfig(ledcfg);

    M5.Led.setLedInstance(led_strip);
    M5.Led.setAutoDisplay(false);
    M5.Led.begin();
    M5.Led.setBrightness(80);

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(&fonts::FreeMonoBold12pt7b);
    M5.Display.setTextSize(1);
    M5.Display.drawString("RGB LED Test", M5.Display.width() / 2,
                         M5.Display.height() / 2);

}

void loop()
{
    M5.update();

    M5.Led.setAllColor(255, 0, 0);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(0, 255, 0);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(0, 0, 255);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(255, 255, 255);
    M5.Led.display();
    delay(1000);

    M5.Led.setAllColor(0, 0, 0);
    M5.Led.display();
    delay(1000);
}
