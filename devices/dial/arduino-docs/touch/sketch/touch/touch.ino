#include <M5Dial.h>

m5::touch_detail_t touchDetail;

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, false, false);  // encoder, RFID

  M5Dial.Display.setTextColor(GREEN);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::FreeMono12pt7b);
  M5Dial.Display.setTextSize(1);

  M5Dial.Display.drawString("Touch Test", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
}

void loop() {
  M5Dial.update();
  touchDetail = M5Dial.Touch.getDetail();

  if (touchDetail.isPressed()) {
    M5.Display.fillCircle(touchDetail.x, touchDetail.y, 2, SKYBLUE);
  }
}
