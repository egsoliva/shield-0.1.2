#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <espnow.h>
#include <SoftwareSerial.h>
#include <math.h>

// GPS setup
SoftwareSerial ss(5, 4);  // TX -> GPIO5 (D1), RX -> GPIO4 (D2)
TinyGPSPlus gps;

uint8_t receiverMAC[] = {0x4E, 0xEB, 0xD6, 0x1F, 0x69, 0xB6};

// Struct message
typedef struct struct_message {
    float lat;
    float lon;
    float alt;
  } struct_message;
  
  struct_message myData;

void setup() {
    Serial.begin(9600);
    ss.begin(9600);
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != 0) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_add_peer(receiverMAC, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    esp_now_register_send_cb(onDataSent);
}

void loop() {
    while (ss.available() > 0) {
        if (gps.encode(ss.read())) {
            if (gps.location.isValid()) {
                float lat = gps.location.lat();
                float lon = gps.location.lng();
                float alt = gps.altitude.meters();
                
                // Send data via ESP-NOW
                esp_now_send(receiverMAC, (uint8_t *)&myData, sizeof(myData));
                
                // Store in struct
                myData.lat = lat;
                myData.lon = lon;
                myData.alt = alt;
            }
        }
    }
}

void onDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    delay(500);
    Serial.print("Send Status: ");
    Serial.println(sendStatus == 0 ? "Success" : "Failed");
}