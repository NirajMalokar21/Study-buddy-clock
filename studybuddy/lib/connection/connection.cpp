#include <Arduino.h>
#include <ArduinoJson.h>
#include "connection.h"
#include "secrets.h"

// Globals
WiFiClient   espClient;
PubSubClient client(espClient);
char         DEVICE_ID[13];

// Helpers
const char* statusToString(BuddyStatus s) {
  switch (s) {
    case BuddyStatus::STUDYING:       return "studying";
    case BuddyStatus::BREAK:          return "break";
    case BuddyStatus::DO_NOT_DISTURB: return "do_not_disturb";
    default:                          return "hang_out";
  }
}

static void initDeviceId() {
  // WiFi.macAddress() reads from eFuse — available immediately after WiFi.begin().
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(DEVICE_ID, sizeof(DEVICE_ID), "%02X%02X%02X%02X%02X%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// WiFi
void wifiConnect() {
  Serial.println("[WiFi] Connecting...");
  WiFi.begin(SSID, PASSWORD);
  initDeviceId();
  Serial.printf("[WiFi] Device ID: %s\n", DEVICE_ID);

  const unsigned long timeout = 10000;
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] Connection failed. Proceeding — MQTT will retry.");
  }
}

// MQTT Publish

// Status messages are RETAINED (flag = true).
// Any device that reconnects or joins late will immediately receive the last
// known status of every peer without waiting for the next publish cycle.
void publishStatus(BuddyStatus status) {
  char topic[64];
  buildTopic(topic, sizeof(topic), "status");

  StaticJsonDocument<128> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["status"]   = statusToString(status);
  doc["name"]     = NAME;
  doc["uptime"]   = millis();  

  char payload[128];
  serializeJson(doc, payload, sizeof(payload));

  client.publish(topic, payload, /*retained=*/true);
  Serial.printf("[MQTT] Status published: %s\n", payload);
}

// Presence messages are RETAINED.
// The LWT (configured in reconnectServer) ensures the broker automatically
// publishes {"online":false} if this device disconnects ungracefully — no
// polling or timeout logic needed on peer devices.
void publishPresence(bool online) {
  char topic[64];
  buildTopic(topic, sizeof(topic), "presence");

  StaticJsonDocument<64> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["online"]   = online;

  char payload[64];
  serializeJson(doc, payload, sizeof(payload));

  client.publish(topic, payload, /*retained=*/true);
  Serial.printf("[MQTT] Presence published: %s\n", payload);
}

// MQTT Connection
void reconnectServer() {
  while (!client.connected()) {
    Serial.printf("[MQTT] Connecting as %s...\n", DEVICE_ID);

    // Last Will and Testament — published by the broker automatically if this
    // device drops off the network without sending a clean DISCONNECT.
    char lwtTopic[64];
    buildTopic(lwtTopic, sizeof(lwtTopic), "presence");
    const char* lwtPayload = "{\"online\":false}";

    // QoS 1 ensures the LWT message is delivered at least once, so peers won't
    const uint8_t lwtQos  = 1;
    const bool    lwtRetain = true;

    if (client.connect(DEVICE_ID,
                       /*user=*/nullptr, /*pass=*/nullptr,
                       lwtTopic, lwtQos, lwtRetain, lwtPayload)) {
      Serial.println("[MQTT] Connected.");

      // Announce presence immediately after connecting.
      publishPresence(true);

      // Wildcard subscriptions — covers all current and future peer devices.
      // QoS 1 ensures at-least-once delivery for status changes.
      client.subscribe(TOPIC_ALL_STATUS,   1);
      client.subscribe(TOPIC_ALL_PRESENCE, 1);

    } else {
      Serial.printf("[MQTT] Failed (state=%d). Retrying in 5s.\n", client.state());
      delay(5000);
    }
  }
}

// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Safely null-terminate the raw payload bytes.
  char msg[256];
  size_t copyLen = min((unsigned int)(sizeof(msg) - 1), length);
  memcpy(msg, payload, copyLen);
  msg[copyLen] = '\0';

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, msg);
  if (err) {
    Serial.printf("[MQTT] Bad JSON on [%s]: %s\n", topic, err.c_str());
    return;
  }

  const char* senderId = doc["deviceId"] | "unknown";

  // Ignore retained echoes of our own messages that arrive on reconnect.
  if (strcmp(senderId, DEVICE_ID) == 0) return;

  // Route by topic suffix.
  if (strstr(topic, "/status")) {
    const char* status = doc["status"] | "unknown";
    Serial.printf("[EVENT] %s → %s\n", senderId, status);
    // TODO: update your display / LEDs here based on peer status

  } else if (strstr(topic, "/presence")) {
    bool online = doc["online"] | false;
    Serial.printf("[EVENT] %s went %s\n", senderId, online ? "online" : "offline");
    // TODO: show online indicator on display
  }
}