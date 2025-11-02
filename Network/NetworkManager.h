#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WiFiCredentials.h"

class NetworkManager {
    private:
        WiFiClientSecure& wiFiClient;
        PubSubClient client;
        std::string ssid;
        std::string password;
        std::string thingName;
        int8_t timeZone;
        uint8_t dst;

    public:
        using ClientCallback = std::function<void(char*, byte*, unsigned int)>;

        NetworkManager(WiFiClientSecure& net, const char* thingName, int8_t timeZone, uint8_t dst);
        void configureWiFi(WiFiCredentials creds);
        bool connectToWiFi();
        void syncTimeWithNTP();
        void configureMQTT(const char *mqttHost, int mqttPort, ClientCallback callback);
        bool connectToMQTT();
        bool loop();
        bool isConnected() { return client.connected(); }
        bool publish(const char* topic, const char* payload);
};

#endif
