#include <Arduino.h>
#include "connection.h"
#include "secrets.h"

// Define global objects 
WiFiClient espClient;
PubSubClient client(espClient);
bool connected = false;

void wifiConnect() {
  Serial.println("Connecting to WiFi.");
  WiFi.begin(SSID, PASSWORD);

  for (int i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected to Buddy.");
      Serial.printf("Buddy IP: %s\n", WiFi.localIP().toString().c_str());
      return;
    }
    delay(500);
  }

  Serial.println("\nBuddy cannot connect to WiFi.");
}

void reconnectServer() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT Connection.");

    if (client.connect("ESP32_B")) {
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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived [");
  Serial.println(topic);
  Serial.println("]");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}