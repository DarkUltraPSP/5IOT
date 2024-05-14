#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

const char *ssid = "Hesias";
const char *password = "bienvenuechezHesias";

const char *mqtt_server = "172.20.63.156";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_AHTX0 aht;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void connectToMQTT()
{
  Serial.print("Connecting to MQTT broker...");
  while (!client.connected())
  {
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      client.subscribe("clement/temperature");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  display.print("Message arrived [");
  display.print(topic);
  display.print("] ");

  for (int i = 0; i < length; i++)
  {
    display.print((char)payload[i]);
  }
  display.println();
}

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello, World!");
  });

  server.begin();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}

void loop()
{
  if (!client.connected())
  {
    connectToMQTT();
  }
  client.loop();

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  client.publish("clement/temperature", String(temp.temperature).c_str());
  client.publish("clement/humidity", String(humidity.relative_humidity).c_str());

  display.clearDisplay();
  display.setCursor(0, 0);

  if (WiFi.status() == WL_CONNECTED)
  {
    display.print("WiFi:" + String(WiFi.SSID()));
    display.println(" ");
    display.print("IP : ");
    display.print(WiFi.localIP());
    display.println(" ");
  }
  else
  {
    display.print("WiFi: Disconnected");
    display.println(" ");
  }

  display.print("Temp: ");
  display.print(temp.temperature);
  display.println(" C");

  display.print("Humidity: ");
  display.print(humidity.relative_humidity);
  display.println("% rH");

  display.display();

  delay(5000);
}