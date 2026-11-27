#include <M5Unified.h>
#include <M5GFX.h>

m5::touch_detail_t touchDetail;
uint16_t color;

void setup() {
  M5.begin();
  M5.Display.setRotation(0);
  M5.Display.setFont(&fonts::DejaVu40);

  color = random(65535);

  Serial.begin(115200);
  Serial.println("Start drawing!");
  M5.Display.print("Start drawing!");
}

void loop() {
  M5.update();
  touchDetail = M5.Touch.getDetail();

  if (touchDetail.isPressed()) {
    Serial.printf("x:%d, y:%d\r\n", touchDetail.x, touchDetail.y);
    color = (color + 5) % 65536;
    M5.Display.fillCircle(touchDetail.x, touchDetail.y, 15, color);
  }
}
