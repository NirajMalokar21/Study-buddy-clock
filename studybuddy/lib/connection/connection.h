#ifndef CONNECTION_H
#define CONNECTION_H

#include <WiFi.h>
#include <PubSubClient.h>

// Status Types
enum class BuddyStatus : uint8_t {
  HANG_OUT       = 0,
  STUDYING       = 1,
  BREAK          = 2,
  DO_NOT_DISTURB = 3
};

const char* statusToString(BuddyStatus s);

// DEVICE_ID is derived from the chip MAC address at startup — unique per board,
// no manual configuration needed when flashing the same firmware to all devices.
extern char DEVICE_ID[13];  // 6-byte MAC as 12 uppercase hex chars + null terminator


// Topic Structure
//
//   studybuddy/devices/<deviceId>/status    — retained, current status payload
//   studybuddy/devices/<deviceId>/presence  — retained, online/offline payload
//
// All devices subscribe to the wildcard forms below, so adding a third or
// fourth device requires zero config changes anywhere.
#define TOPIC_ALL_STATUS   "studybuddy/devices/+/status"
#define TOPIC_ALL_PRESENCE "studybuddy/devices/+/presence"

inline void buildTopic(char* out, size_t len, const char* sub) {
  snprintf(out, len, "studybuddy/devices/%s/%s", DEVICE_ID, sub);
}

// 
// Globals
extern WiFiClient   espClient;
extern PubSubClient client;

// Functions
void wifiConnect();
void reconnectServer();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// Call these whenever local state changes or on a periodic heartbeat.
void publishStatus(BuddyStatus status);
void publishPresence(bool online);

#endif