#include <M5Unified.h>

void setup(void) {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextFont(&fonts::FreeSerifBold9pt7b);
  M5.Display.setTextSize(1);

  M5.Display.drawString("Press BtnA to Sleep", M5.Display.width() / 2, M5.Display.height() / 2 - 20);

  //RTC wakeup must use following code
  // m5::rtc_time_t time = {  9 , 0 , 55};//hour,min,sec  
  // m5::rtc_date_t date = {2025, 1 ,  1 , 1};//year,mon,date,wed
  // M5.Rtc.setDate(date);
  // M5.Rtc.setTime(time);
}

void loop(void) {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    M5.Display.clear();

    M5.Power.timerSleep(5);//sec
    // m5::rtc_time_t time = {9, 1, 0};
    // m5::rtc_date_t date = {2025, 1 ,  1 , 1};
    // M5.Power.timerSleep(time);
    // M5.Power.timerSleep(date, time);
    // M5.Power.powerOff(); 
  }
}
