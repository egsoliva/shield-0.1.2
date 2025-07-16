#include <ESP8266WiFi.h>
#include <espnow.h>

typedef struct struct_message {
    float lat;
    float lon;
    float alt;
} struct_message;

struct_message myData;

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.println("Data received!");
    Serial.print("Latitude: ");
    Serial.println(myData.lat, 6);
    Serial.print("Longitude: ");
    Serial.println(myData.lon, 6);
    Serial.print("Altitude: ");
    Serial.println(myData.alt);
}

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("Waiting for data...");
}

void loop() {
}