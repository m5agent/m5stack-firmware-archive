#include <M5StickC.h>

RTC_DateTypeDef DateStruct;
void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
 
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(40, 0, 2);
  M5.Lcd.println("RTC TEST");
 
  DateStruct.WeekDay = 3;
  DateStruct.Month = 3;
  DateStruct.Date = 22;
  DateStruct.Year = 2019;
  M5.Rtc.SetData(&DateStruct);
}

void loop() {
  M5.Rtc.GetData(&DateStruct);
  M5.Lcd.setCursor(0, 15);
  M5.Lcd.printf("Date:%04d-%02d-%02d/n",DateStruct.Year, DateStruct.Month,DateStruct.Date);
  M5.Lcd.printf("Week:%d/n",DateStruct.WeekDay);
  delay(500);
}
