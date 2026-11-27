#include <Arduino.h>
#include <M5Unified.h>

void setup()
{
    /* Init M5 */
    M5.begin();

    M5.Display.setTextScroll(true);
    M5.Display.setTextColor(TFT_GREENYELLOW);
    M5.Display.println("Button example");
    M5.Display.setTextColor(TFT_YELLOW);
}

void loop()
{
    /* Update button states */
    M5.update();

    /* Check if button was clicked */
    if (M5.BtnA.wasClicked()) {
        M5.Display.println("Button A was clicked");
    } else if (M5.BtnB.wasClicked()) {
        M5.Display.println("Button B was clicked");
    } else if (M5.BtnC.wasClicked()) {
        M5.Display.println("Button C was clicked");
    }

    delay(100);
}
