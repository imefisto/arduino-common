#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <PubSubClient.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif
#include <WiFiClientSecure.h>
#include <string>

class NetworkManager {
private:
    WiFiClientSecure net;
    PubSubClient client;
    std::string ssid;
    std::string password;
    std::string thingName;
    int8_t timeZone;
    uint8_t dst;
    
    void connectToWiFi();
    void connectToMqtt();
    void connectToNTP();
    
public:
    NetworkManager(const char* ssid, const char* password, const char* thingName, int8_t timeZone, uint8_t dst);
    void setup();
    void loop();
    bool isConnected() { return client.connected(); }
    bool publish(const char* topic, const char* payload);
    void setCallback(void (*callback)(char*, byte*, unsigned int));
    PubSubClient& getMqttClient() { return client; }
};

#endif
