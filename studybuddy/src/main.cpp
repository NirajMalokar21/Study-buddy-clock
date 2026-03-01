#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <PubSubClient.h>

// client info
WiFiClient espClient;
PubSubClient client(espClient);
bool connected = false;
const int baudRate = 115200;

// function declarations
void callback(char* topic, byte* payload, unsigned int length);
void reconnectServer(); 
void wifiConnect();

void wifiConnect(){
  Serial.println("Connecting to WiFi.");
  WiFi.begin(SSID, PASSWORD);

  for (int i = 0; i < 10; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi connected to Buddy.");
      Serial.printf("Buddy IP: %s\n", WiFi.localIP().toString().c_str());
      return;
    }
    delay(500);
  }
  Serial.println("\nBuddy cannot connect to WiFi.");
}

void reconnectServer(){
  while (client.connected() != true)
  {
    Serial.println("Attempting MQTT Connection.");
    
    if (client.connect("ESP32_B") == true)
    {
      Serial.println("Connected to MQTT Server.");
      connected = true;

      client.subscribe("studybuddy/B/buddy");
    } else {
      Serial.println("Failed connection. Reconnecting.");
      Serial.print(client.state());
      Serial.println("Retrying in 5 seconds.");

      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.println("Message arrived [");
  Serial.println(topic);
  Serial.println("]");

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  // baudRate for serial monitor
  Serial.begin(baudRate);
  delay(2000);

  Serial.println("Buddy initiated");
  wifiConnect();
  client.setServer(SERVER_IP, PORT);
  client.setCallback(callback);
}

void loop() {
  if (client.connected() != true)
  {
    reconnectServer();
  }

  // keeping MQTT active
  client.loop(); 
  
  // Example: Publish every 5 seconds
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000)
  {
    lastMsg = millis();
    client.publish("studybuddy/B/state", "{ \"state\": \"THIS IS MAHIN\" }");
  }
}
