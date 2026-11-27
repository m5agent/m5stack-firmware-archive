#include "OThreadCLI.h"
#include "OThreadCLI_Util.h"

#define CLI_NETWORK_KEY    "dataset networkkey 00112233445566778899aabbccddeeff"
#define CLI_NETWORK_CHANEL "dataset channel 24"

otInstance *aInstance = NULL;

bool udpInitialized = false;

void setup() {
  Serial.begin(115200);
  OThread.begin(false);
  OThreadCLI.begin();
  Serial.println();
  Serial.println("Setting up OpenThread Node as Leader");
  aInstance = esp_openthread_get_instance();

  OThreadCLI.println("dataset init new");
  OThreadCLI.println(CLI_NETWORK_KEY);
  OThreadCLI.println(CLI_NETWORK_CHANEL);
  OThreadCLI.println("dataset commit active");
  OThreadCLI.println("ifconfig up");
  OThreadCLI.println("thread start");
}

void loop() {
  while (OThreadCLI.available()) {
    Serial.write(OThreadCLI.read());
  }

  Serial.println("=============================================");
  Serial.print("Thread Node State: ");
  Serial.println(OThread.otGetStringDeviceRole());

  if (OThread.otGetDeviceRole() == OT_ROLE_LEADER) {
    const char *networkName = otThreadGetNetworkName(aInstance);
    Serial.printf("Network Name: %s\r\n", networkName);
    uint8_t channel = otLinkGetChannel(aInstance);
    Serial.printf("Channel: %d\r\n", channel);
    uint16_t panId = otLinkGetPanId(aInstance);
    Serial.printf("PanID: 0x%04x\r\n", panId);
    const otExtendedPanId *extPanId = otThreadGetExtendedPanId(aInstance);
    Serial.printf("Extended PAN ID: ");
    for (int i = 0; i < OT_EXT_PAN_ID_SIZE; i++) {
      Serial.printf("%02x", extPanId->m8[i]);
    }
    Serial.println();
    otNetworkKey networkKey;
    otThreadGetNetworkKey(aInstance, &networkKey);
    Serial.printf("Network Key: ");
    for (int i = 0; i < OT_NETWORK_KEY_SIZE; i++) {
      Serial.printf("%02x", networkKey.m8[i]);
    }
    Serial.println();
    char buf[OT_IP6_ADDRESS_STRING_SIZE];
    const otNetifAddress *address = otIp6GetUnicastAddresses(aInstance);
    while (address != NULL) {
      otIp6AddressToString(&address->mAddress, buf, sizeof(buf));
      Serial.printf("IP Address: %s\r\n", buf);
      address = address->mNext;
    }
    const otNetifMulticastAddress *mAddress = otIp6GetMulticastAddresses(aInstance);
    while (mAddress != NULL) {
      otIp6AddressToString(&mAddress->mAddress, buf, sizeof(buf));
      printf("Multicast IP Address: %s\n", buf);
      mAddress = mAddress->mNext;
    }

    if (!udpInitialized) {
      Serial.println("\nInitializing UDP sender...");
      OThreadCLI.println("udp open");
      delay(100);
      udpInitialized = true;
      Serial.println("UDP initialized");
    }

    Serial.println("\nSending UDP message...");
    OThreadCLI.println("udp send ff03::1 12345 \"Hello,M5Stack\"");
  }

  delay(5000);
}
