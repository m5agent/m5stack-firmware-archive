#include <M5StickC.h>

RTC_TimeTypeDef TimeStruct;
RTC_DateTypeDef DateStruct;
void setup() {
  M5.begin();

  DateStruct.WeekDay = 3;
  DateStruct.Month = 3;
  DateStruct.Date = 22;
  DateStruct.Year = 2019;
  M5.Rtc.SetData(&DateStruct);
}
void loop(){};
