#define WIFI_SSID     "********"
#define WIFI_PASSWORD "********"

#define NTP_TIMEZONE "UTC-8"  // POSIX standard, in which "UTC+0" is UTC London, "UTC-8" is UTC+8 Beijing, "UTC+5" is UTC-5 New York
#define NTP_SERVER1  "0.pool.ntp.org"
#define NTP_SERVER2  "1.pool.ntp.org"
#define NTP_SERVER3  "2.pool.ntp.org"
#include <WiFi.h>

// Different versions of the framework have different SNTP header file names and availability.
#if __has_include(<esp_sntp.h>)
#include <esp_sntp.h>
#define SNTP_ENABLED 1
#elif __has_include(<sntp.h>)
#include <sntp.h>
#define SNTP_ENABLED 1
#endif

#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

#include <M5Dial.h>

static constexpr const char* const wd[7] = { "Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat" };

void setup() {
  M5Dial.begin();
  M5Dial.Display.setFont(&fonts::FreeMono9pt7b);
  M5Dial.Display.setCursor(0, 60);
  M5Dial.Display.println("RTC Test");

  if (!M5Dial.Rtc.isEnabled()) {
    M5Dial.Display.println("RTC not found");
    while (true) {
      delay(500);
    }
  }
  M5Dial.Display.println("RTC found");

  M5Dial.Display.print("WiFi: ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    M5Dial.Display.print(".");
    delay(500);
  }
  M5Dial.Display.println("\nWiFi connected");

  configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

  M5Dial.Display.print("NTP: ");
#if SNTP_ENABLED
  while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
    M5Dial.Display.print(".");
    delay(500);
  }
#else
  struct tm timeInfo;
  while (!getLocalTime(&timeInfo, 1000)) {
    M5Dial.Display.print(".");
    delay(500);
  }
#endif
  M5Dial.Display.println("\nNTP connected");

  time_t t = time(nullptr) + 1;  // Advance one second
  while (t > time(nullptr))
    ;  // Synchronization in seconds
  M5Dial.Rtc.setDateTime(gmtime(&t));

  delay(1000);
  M5Dial.Display.clear();
  M5Dial.Display.setCursor(70, 20);
  M5Dial.Display.println("RTC Test");
}

void loop() {
  auto dt = M5Dial.Rtc.getDateTime();
  M5Dial.Display.setCursor(40, 50);
  M5Dial.Display.printf("RTC   UTC  :");
  M5Dial.Display.setCursor(40, 65);
  M5Dial.Display.printf("%04d/%02d/%02d(%s)", dt.date.year, dt.date.month, dt.date.date, wd[dt.date.weekDay]);
  M5Dial.Display.setCursor(40, 80);
  M5Dial.Display.printf("%02d:%02d:%02d", dt.time.hours, dt.time.minutes, dt.time.seconds);

  // ESP32 internal timer
  auto t = time(nullptr);

  {
    auto tm = gmtime(&t);  // for UTC
    M5Dial.Display.setCursor(40, 110);
    M5Dial.Display.printf("ESP32 UTC  :");
    M5Dial.Display.setCursor(40, 125);
    M5Dial.Display.printf("%04d/%02d/%02d(%s)", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday]);
    M5Dial.Display.setCursor(40, 140);
    M5Dial.Display.printf("%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  }

  {
    auto tm = localtime(&t);  // for local timezone
    M5Dial.Display.setCursor(40, 170);
    M5Dial.Display.printf("ESP32 %s:", NTP_TIMEZONE);
    M5Dial.Display.setCursor(40, 185);
    M5Dial.Display.printf("%04d/%02d/%02d(%s)", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday]);
    M5Dial.Display.setCursor(40, 200);
    M5Dial.Display.printf("%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  }

  delay(500);
}
