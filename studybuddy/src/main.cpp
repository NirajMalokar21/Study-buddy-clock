#include <Arduino.h>
#include "connection.h"
#include "secrets.h"

// Local State

// The device's current status. Update this from button presses or any other
// input and call publishStatus() — the broker fans it out to all peers.
static BuddyStatus currentStatus = BuddyStatus::HANG_OUT;

// How often to re-publish status as a heartbeat (ms).
// Keeps retained state fresh if the broker restarts and loses retained data.
static const unsigned long HEARTBEAT_MS = 30000;

// Setup
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== Study Buddy Starting ===");

  wifiConnect();

  // Increase the PubSubClient buffer from its 256-byte default.
  client.setBufferSize(512);
  client.setServer(SERVER_IP, PORT);
  client.setCallback(mqttCallback);

  reconnectServer();

  // Publish initial status so peers see this device immediately.
  publishStatus(currentStatus);
}

// Loop
void loop() {
  if (!client.connected()) {
    reconnectServer();
    publishStatus(currentStatus);  // Re-announce after reconnect
  }

  client.loop();

  // Heartbeat
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > HEARTBEAT_MS) {
    lastHeartbeat = millis();
    publishStatus(currentStatus);
  }

  // TODO: Physical input handling would go here
}