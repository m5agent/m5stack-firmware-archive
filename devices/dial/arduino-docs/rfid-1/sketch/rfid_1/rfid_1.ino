#include "M5Dial.h"

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, false, true);  // encoder, RFID
  Serial.begin(115200);
}

void loop() {
  // PICC: Proximity Integrated Circuit Card
  if (M5Dial.Rfid.PICC_IsNewCardPresent() && M5Dial.Rfid.PICC_ReadCardSerial()) {
    M5Dial.Display.clear();

    uint8_t piccType = M5Dial.Rfid.PICC_GetType(M5Dial.Rfid.uid.sak);
    Serial.print(F("PICC type: "));
    Serial.println(M5Dial.Rfid.PICC_GetTypeName(piccType));

    // Check if the tag / card is of type MIFARE Classic
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("This tag / card is not of type MIFARE Classic.\n"));
      delay(500);
      return;
    }

    // Output the stored UID data
    for (byte i = 0; i < M5Dial.Rfid.uid.size; i++) {
      Serial.printf("%02X ", M5Dial.Rfid.uid.uidByte[i]);
    }
    Serial.println("\n");
    delay(500);
  }
}
