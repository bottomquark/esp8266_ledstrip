#include "networking.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

uint32_t mqttReconnectAttempt = 0;
uint32_t wifiReconnectAttempt = 0;

void networkingSetup(MQTT_CALLBACK_SIGNATURE)
{
  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
}

void networkingLoop()
{
  if (WiFi.status() == WL_CONNECTED) {
    mqttClientLoop();
  }
  else {
    setupWifi();
  }
}

void setupWifi() {

  uint32_t now = millis();
  if (now - wifiReconnectAttempt > 10000) {
    wifiReconnectAttempt = now;
    //need to disconnect completely and then set WIFI_STA (otherwise we are also in AP mode and therefore cannot connect to IPs 192.168.4.x)
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

boolean mqttReconnect() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    // ... and resubscribe
    mqttClient.subscribe(IN_SUBSCRIBE_TOPIC "/#");
  }
  return mqttClient.connected();
}

void mqttClientLoop() {
  //loop the MQTT client
  if (!mqttClient.connected()) {
    uint32_t now = millis();
    if (now - mqttReconnectAttempt > 5000) {
      mqttReconnectAttempt = now;
      // Attempt to reconnect
      if (mqttReconnect()) {
        mqttReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    mqttClient.loop();
  }

}

// vim:ai:cin:sts=2 sw=2 ft=cpp
