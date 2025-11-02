#ifndef ACCESS_POINT_MANAGER_H
#define ACCESS_POINT_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include "WiFiCredentials.h"

class AccessPointManager {
    public:
        using CredentialsSavedCallback = std::function<void(WiFiCredentials creds)>;

        AccessPointManager(const char* ssid = "ESP8266-Config", 
                const char* apPassword = "12345678",
                uint32_t timeout = 300000); // 5 minutes default

        bool start();
        IPAddress getIPAddress() { return ip; }

        void stop();
        void onCredentialsSaved(CredentialsSavedCallback callback) {
            onCredentialsSavedCallback = callback;
        }

        bool hasTimedOut() const { return timeout && (millis() - startTime >= timeout); }

    private:
        void setupWebServer();
        void handleRoot(AsyncWebServerRequest *request);
        void handleSave(AsyncWebServerRequest *request);

        const char* ssid;
        const char* password;
        const uint32_t timeout;
        unsigned long startTime;
        bool started = false;
        IPAddress ip;

        AsyncWebServer server;
        CredentialsSavedCallback onCredentialsSavedCallback;
};

#endif
