#define NTP_TIMEZONE "UTC-8"
#define NTP_SERVER1  "0.pool.ntp.org"
#define NTP_SERVER2  "1.pool.ntp.org"
#define NTP_SERVER3  "2.pool.ntp.org"

#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <sntp.h>

const char *ssid     = "ssid";
const char *password = "password";

void setup(void)
{
    M5.begin();
    Serial.begin(115200);
    M5.Display.setFont(&fonts::FreeMonoBold12pt7b);

    // STA MODE
    WiFi.mode(WIFI_STA);
    M5.Display.setCursor(40, 100);
    M5.Display.println("WiFi mode set to STA");
    WiFi.begin(ssid, password);
    M5.Display.setCursor(40, 130);
    M5.Display.print("Connecting to ");
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.print(".");
    }
    M5.Display.println("");
    M5.Display.setCursor(40, 160);
    M5.Display.print("Connected to ");
    M5.Display.println(ssid);
    M5.Display.setCursor(40, 190);
    M5.Display.print("IP address: ");
    M5.Display.println(WiFi.localIP());
    M5.Display.setCursor(40, 220);

    if (!M5.Rtc.isEnabled()) {
        Serial.println("RTC not found.");
        M5.Display.println("RTC not found.");
        for (;;) {
            vTaskDelay(500);
        }
    }

    M5.Display.println("RTC found.");
    Serial.println("RTC found.");
    M5.Display.setCursor(40, 250);
    M5.Display.println("SNTP Sync...");

    configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        Serial.print('.');
        delay(1000);
    }

    Serial.println("\r\n NTP Connected.");

    time_t t = time(nullptr) + 1;  // Advance one second.
    while (t > time(nullptr));     /// Synchronization in seconds
    M5.Rtc.setDateTime(gmtime(&t));
    M5.Display.clear();
}

void loop(void)
{
    static constexpr const char *const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};

    delay(500);

    auto dt = M5.Rtc.getDateTime();
    Serial.printf("RTC  UTC: %04d/%02d/%02d (%s)  %02d:%02d:%02d\r\n", dt.date.year, dt.date.month, dt.date.date,
                  wd[dt.date.weekDay], dt.time.hours, dt.time.minutes, dt.time.seconds);
    M5.Display.setCursor(40, 100);
    M5.Display.printf("RTC UTC: %04d/%02d/%02d (%s)                 %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.date,
                      wd[dt.date.weekDay], dt.time.hours, dt.time.minutes, dt.time.seconds);

    /// ESP32 internal timer
    auto t = time(nullptr);
    {
        auto tm = gmtime(&t);  // for UTC.
        Serial.printf("ESP32 UTC: %04d/%02d/%02d (%s)  %02d:%02d:%02d\r\n", tm->tm_year + 1900, tm->tm_mon + 1,
                      tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
        M5.Display.setCursor(40, 160);
        M5.Display.printf("ESP32 UTC: %04d/%02d/%02d (%s)                 %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1,
                          tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
    }

    {
        auto tm = localtime(&t);  // for local timezone.
        Serial.printf("ESP32 %s: %04d/%02d/%02d (%s)  %02d:%02d:%02d\r\n", NTP_TIMEZONE, tm->tm_year + 1900,
                      tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
        M5.Display.setCursor(40, 220);
        M5.Display.printf("ESP32 %s: %04d/%02d/%02d (%s)                 %02d:%02d:%02d", NTP_TIMEZONE, tm->tm_year + 1900,
                          tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
}
