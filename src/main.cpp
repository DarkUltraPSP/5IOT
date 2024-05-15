/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t broadcastAddress[] = {0xEC, 0xDA, 0x3B, 0xBF, 0x0A, 0x7C};

Adafruit_AHTX0 aht;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message_t;

// Create a struct_message called myData
struct_message_t myData;

esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Temperature: ");
  Serial.println(myData.temperature);
  Serial.print("Humidity:");
  Serial.println(myData.humidity);

  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("Temp: ");
  display.print(myData.temperature);
  display.println(" C");

  display.print("Humidity: ");
  display.print(myData.humidity);
  display.println("% rH");

  display.display();
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Wire.begin(3, 2);

  if (!aht.begin(&Wire))
  {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }
  Serial.println("AHT10 found");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  // Set values to send
  myData.temperature = temp.temperature;
  myData.humidity = humidity.relative_humidity;

  // Serial.print("Temperature: ");
  // Serial.print(myData.temperature);
  // Serial.print(" °C");
  // Serial.print("\tHumidity: ");
  // Serial.println(myData.humidity);
  
  // Send message via ESP-NOW
  // esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  // if (result == ESP_OK) {
  //   Serial.println("Sent with success");
  // }
  // else {
  //   Serial.println("Error sending the data");
  // }

  delay(1000);
}