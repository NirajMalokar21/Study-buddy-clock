#ifndef CONNECTION_H
#define CONNECTION_H

#include <WiFi.h>
#include <PubSubClient.h>

// Extern so they are accessible from other files
extern WiFiClient espClient;
extern PubSubClient client;
extern bool connected;

// Function declarations
void wifiConnect();
void reconnectServer();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif