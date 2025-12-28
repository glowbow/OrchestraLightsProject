#include <WiFi.h>
#include <esp_now.h>

#define BROADCAST_ADDRESS_5 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} //general mac address for all receivers

typedef struct {
  int groupId; //ID for addressing individual lights or groups at a time
  int number; //key num displayed in monitor
  int value; //the key value either 127 or 0 for a general key or a varying value for a slider
  uint32_t messageId; //identify each message
} MidiData;

MidiData receivedData;
uint32_t messageIdCounter = 0;
esp_now_peer_info_t peerInfo1, peerInfo2, peerInfo3, peerInfo4, peerInfo5;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { //did send succeed?
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Send status: Success");
  } else {
    Serial.println("Send status: Failed");
  }
}

bool parseSerialData(String input) { //reads messages from the Serial Monitor and places the vars into a struct
  int commaIndex = input.indexOf(',');
  if (commaIndex > 0) {
    receivedData.number = input.substring(0, commaIndex).toInt();
    receivedData.value = input.substring(commaIndex + 1).toInt();
    return true;
  }
  return false;
}

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);



uint8_t broadcastAddr5[6] = BROADCAST_ADDRESS_5; //the general address
  memcpy(peerInfo5.peer_addr, broadcastAddr5, sizeof(peerInfo5.peer_addr));
  peerInfo5.channel = 0;
  peerInfo5.encrypt = false;
  esp_now_add_peer(&peerInfo5);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    if (parseSerialData(input)) {
          receivedData.messageId = messageIdCounter++;
      if (receivedData.number == 78) { //message is sent out to groups in sequence 1->4
          for (int i = 1; i < 5; i++) {
            receivedData.groupId = i;
            Serial.printf("Sending: Number=%d, Group=%d, MessageId=%d\n", receivedData.number, receivedData.groupId, receivedData.messageId);
            Serial.print("Sent to: ");
            Serial.println(i);
            esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
            delay(300);
          }

        } else if (receivedData.number == 82) { //message is sent out to groups in sequence 4->1
          for (int i = 4; i > 0; i--) {
            receivedData.groupId = i;
            Serial.printf("Sending: Number=%d, Group=%d, MessageId=%d\n", receivedData.number, receivedData.groupId, receivedData.messageId);
            Serial.print("Sent to: ");
            Serial.println(i);
            esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
            delay(300);
          }
          } else if (receivedData.number == 80) { //message is sent out to groups in sequence 1 3 2 4
              for (int i = 0; i < 2; i++) {
              receivedData.groupId = 1;
              esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
              receivedData.groupId = 3;
              esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
              delay(400);
              receivedData.groupId = 2;
              esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
              receivedData.groupId = 4;
              esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
              delay(400);
              }
        } else { //message is sent to all, same time
          receivedData.groupId = 0;
          Serial.println("sending to the shared address of everyone");
          esp_now_send(peerInfo5.peer_addr, (uint8_t *)&receivedData, sizeof(receivedData));
        }
    }
  }
}