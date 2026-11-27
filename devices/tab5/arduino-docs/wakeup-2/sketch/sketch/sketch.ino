#include <M5Unified.h>

m5::rtc_time_t wakeTime = {};

void setup(void) {
    auto cfg = M5.config();
    M5.begin(cfg);

    if (M5.Rtc.isEnabled()) {
        // Set an initial RTC time for testing.
        // Format: {{year, month, date, weekDay}, {hour, minute, second}}
        m5::rtc_datetime_t initialDateTime = {{2026, 1, 1, 4}, {12, 0, 0}};
        M5.Rtc.setDateTime(&initialDateTime.date, &initialDateTime.time);
    }

    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::FreeMonoBold18pt7b);
    M5.Display.setRotation(1);

    M5.Display.clear();

    M5.Display.drawString("Touch to sleep", M5.Display.width() / 2, M5.Display.height() / 2 - 20);
    M5.Display.drawString("After about 1min wakeup", M5.Display.width() / 2, M5.Display.height() / 2 + 20);
}

void loop(void) {
    M5.update();

    if (M5.Touch.getDetail(0).wasClicked()) {
        m5::rtc_time_t now;

        if (M5.Rtc.getTime(&now)) {
            int wakeHour = now.hours;
            int wakeMinute = now.minutes + 1;

            if (wakeMinute >= 60) {
                wakeMinute -= 60;
                wakeHour += 1;
                if (wakeHour >= 24) {
                    wakeHour = 0;
                }
            }

            wakeTime.hours = wakeHour;
            wakeTime.minutes = wakeMinute;
            wakeTime.seconds = -1;  // RX8130 alarm ignores seconds in this library.

            M5.Power.timerSleep(wakeTime);
        } else {
            M5.Display.clear();
            M5.Display.drawString("RTC Error", M5.Display.width() / 2, M5.Display.height() / 2);
        }
    }
    delay(10);
}
