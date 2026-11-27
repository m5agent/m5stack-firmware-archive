#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <sntp.h>

#define NTP_TIMEZONE "UTC-8"
#define NTP_SERVER1  "0.pool.ntp.org"
#define NTP_SERVER2  "1.pool.ntp.org"
#define NTP_SERVER3  "2.pool.ntp.org"

static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;
static constexpr uint32_t SNTP_SYNC_TIMEOUT_MS    = 30000;

M5Canvas canvas(&M5.Display);

const char* ssid     = "ssid";
const char* password = "password";

static constexpr const char* const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static int last_drawn_minute = -1;

void drawScreen(const m5::rtc_datetime_t& dt)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    char dateText[32];
    char timeText[16];
    snprintf(dateText, sizeof(dateText), "%04d/%02d/%02d", dt.date.year, dt.date.month, dt.date.date);
    snprintf(timeText, sizeof(timeText), "%02d:%02d", dt.time.hours, dt.time.minutes);

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextColor(TFT_BLACK);
    canvas.setTextDatum(top_center);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString("PaperMono RTC Test", w / 2, 24);
    canvas.drawString("SNTP", w / 2, 70);

    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.setTextSize(1);
    canvas.drawString(timeText, w / 2, h / 2 - 80);

    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString(dateText, w / 2, h / 2 + 48);
    canvas.drawString(wd[dt.date.weekDay], w / 2, h / 2 + 94);

    canvas.pushSprite(0, 0);
}

void drawSyncScreen()
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_BLACK);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString("PaperMono RTC Test", w / 2, h / 2 - 72);
    canvas.drawString("SNTP Sync....", w / 2, h / 2);
    canvas.setFont(&fonts::FreeSansBold12pt7b);
    canvas.drawString("Please wait", w / 2, h / 2 + 52);
    canvas.pushSprite(0, 0);
}

void drawErrorScreen(const char* message)
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_BLACK);
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.drawString("PaperMono RTC Test", w / 2, h / 2 - 48);
    canvas.drawString(message, w / 2, h / 2 + 36);
    canvas.pushSprite(0, 0);
}

bool connectWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    const uint32_t start_ms = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start_ms >= WIFI_CONNECT_TIMEOUT_MS) {
            return false;
        }
        delay(250);
    }

    Serial.printf("WiFi connected, IP: %s\r\n", WiFi.localIP().toString().c_str());
    return true;
}

bool syncRtcFromSntp()
{
    Serial.println("SNTP sync start...");
    configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    const uint32_t start_ms = millis();
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        if (millis() - start_ms >= SNTP_SYNC_TIMEOUT_MS) {
            return false;
        }
        delay(500);
    }

    time_t now = time(nullptr) + 1;
    while (now > time(nullptr)) {
        delay(10);
    }

    auto local_tm = localtime(&now);
    if (local_tm == nullptr) {
        return false;
    }

    M5.Rtc.setDateTime(local_tm);
    Serial.printf("RTC synced from SNTP: %04d/%02d/%02d %02d:%02d:%02d (%s)\r\n",
                  local_tm->tm_year + 1900,
                  local_tm->tm_mon + 1,
                  local_tm->tm_mday,
                  local_tm->tm_hour,
                  local_tm->tm_min,
                  local_tm->tm_sec,
                  NTP_TIMEZONE);
    return true;
}

void setup()
{
    auto cfg = M5.config();
    cfg.clear_display = false;
    M5.begin(cfg);
    Serial.begin(115200);
    M5.Display.setRotation(0);
    M5.Display.setEpdMode(epd_mode_t::epd_text);

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawSyncScreen();

    if (!M5.Rtc.isEnabled()) {
        Serial.println("RTC not found.");
        drawErrorScreen("RTC not found");
        while (true) {
            delay(1000);
        }
    }

    if (!connectWiFi()) {
        Serial.println("WiFi connect failed.");
        drawErrorScreen("WiFi failed");
        while (true) {
            delay(1000);
        }
    }

    if (!syncRtcFromSntp()) {
        Serial.println("SNTP sync failed.");
        drawErrorScreen("SNTP failed");
        while (true) {
            delay(1000);
        }
    }

    const auto dt = M5.Rtc.getDateTime();
    drawScreen(dt);
    last_drawn_minute = dt.time.minutes;
}

void loop()
{
    M5.update();
    const auto dt = M5.Rtc.getDateTime();
    if (dt.time.minutes != last_drawn_minute) {
        last_drawn_minute = dt.time.minutes;
        drawScreen(dt);
    }
    delay(100);
}
