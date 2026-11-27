#include <M5StickC.h>

RTC_TimeTypeDef TimeStruct;
void setup() {
  M5.begin();

  TimeStruct.Hours   = 18;
  TimeStruct.Minutes = 56;
  TimeStruct.Seconds = 10;
  M5.Rtc.SetTime(&TimeStruct);
}
void loop(){};
