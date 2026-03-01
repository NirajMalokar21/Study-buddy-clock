#include <Arduino.h>
#include "connection.h"
#include "secrets.h"
#include "OLED.h"


const int baudRate = 115200;

void setup() {
  Serial.begin(baudRate);
  delay(2000);

  Serial.println("Buddy initiated");

  wifiConnect();

  client.setServer(SERVER_IP, PORT);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnectServer();
  }

  client.loop();

  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    client.publish("studybuddy/B/state", "{ \"state\": \"THIS IS MAHIN\" }");
  }
}