#include "M5Dial.h"

MFRC522::MIFARE_Key key;

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, false, true);  // encoder, RFID
  Serial.begin(115200);

  // Prepare the key (used both as key A and as key B) with FFFFFFFFFFFFh,
  // which is the default at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

// Helper routine to dump a byte array as hex values to Serial.
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void loop() {
  M5Dial.update();

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
    String uid = "";
    for (byte i = 0; i < M5Dial.Rfid.uid.size; i++) {
      Serial.printf("%02X ", M5Dial.Rfid.uid.uidByte[i]);
      uid += String(M5Dial.Rfid.uid.uidByte[i], HEX);
    }
    Serial.println("\n");

    // M5Dial.Rfid.PICC_DumpToSerial(&(M5Dial.Rfid.uid));
    // Serial.println("\n");

    // In this example, we use the second sector (sector #1) including blocks #4, #5, #6, #7
    byte sector = 1;
    byte blockAddr = 4;
    byte trailerBlock = 7;
    byte dataBlock[] = {
      0x01, 0x02, 0x03, 0x04,  //  1,  2,  3,   4,
      0x05, 0x06, 0x07, 0x08,  //  5,  6,  7,   8,
      0x09, 0x0a, 0x0b, 0x0c,  //  9, 10, 11,  12,
      0x0d, 0x0e, 0x0f, 0xff   // 13, 14, 15, 255
    };
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode)M5Dial.Rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(M5Dial.Rfid.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(M5Dial.Rfid.GetStatusCodeName(status));
      return;
    }

    // Show the whole sector as it currently is
    Serial.println(F("Current data in sector: "));
    M5Dial.Rfid.PICC_DumpMifareClassicSectorToSerial(&(M5Dial.Rfid.uid), &key, sector);
    Serial.println("");

    // Read data from the block
    Serial.print(F("Reading data from block #"));
    Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode)M5Dial.Rfid.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(M5Dial.Rfid.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block #"));
    Serial.print(blockAddr);
    Serial.println(F(": "));
    dump_byte_array(buffer, 16);
    Serial.println("\n");

    // Authenticate using key B
    Serial.println(F("Authenticating again using key B..."));
    status = (MFRC522::StatusCode)M5Dial.Rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(M5Dial.Rfid.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(M5Dial.Rfid.GetStatusCodeName(status));
      return;
    }

    // Write data to the block
    Serial.print(F("Writing data into block #"));
    Serial.print(blockAddr);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 16);
    Serial.println();
    status = (MFRC522::StatusCode)M5Dial.Rfid.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(M5Dial.Rfid.GetStatusCodeName(status));
    }
    Serial.println("");

    // Read data from the block again, now it should be what we have written
    Serial.print(F("Reading data from block #"));
    Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode)M5Dial.Rfid.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(M5Dial.Rfid.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block #"));
    Serial.print(blockAddr);
    Serial.println(F(": "));
    dump_byte_array(buffer, 16);
    Serial.println("\n");

    // Check if the data in block is what we have written, by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
      // Compare buffer (what we've read) with dataBlock (what we've written)
      if (buffer[i] == dataBlock[i]) {
        count++;
      }
    }
    Serial.print(F("Number of bytes that match = "));
    Serial.println(count);
    if (count == 16) {
      Serial.println(F("Success :-)"));
    } else {
      Serial.println(F("Failure, no match :-("));
      Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();

    // Dump the sector data
    Serial.println(F("Current data in sector: "));
    M5Dial.Rfid.PICC_DumpMifareClassicSectorToSerial(&(M5Dial.Rfid.uid), &key, sector);
    Serial.println("");

    // Halt PICC
    M5Dial.Rfid.PICC_HaltA();
    // Stop encryption on PCD
    M5Dial.Rfid.PCD_StopCrypto1();

    Serial.println("====================");
  }
}
