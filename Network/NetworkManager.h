#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <functional>
#include <string>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif
#include "../WiFiCredentials/WiFiCredentials.h"
#include "../WiFiCredentials/WiFiCredentialsManager.h"
#include "../WiFiCredentials/AccessPointManager.h"

class NetworkManager {
public:
    using MqttCallback = std::function<void(char*, byte*, unsigned int)>;

    NetworkManager(const char* thingName,
                   const char* mqttHost, int mqttPort,
                   const char* caCert, const char* clientCert, const char* privateKey,
                   int8_t timeZone, uint8_t dst);

    void begin(MqttCallback callback);
    void loop();

    bool isConnected() { return _client.connected(); }
    bool publish(const char* topic, const char* payload);
    void clearCredentials() { _credManager.clearCredentials(); }

private:
    enum State { ST_BOOT, ST_TRY_CONNECT, ST_CONNECTED, ST_AP_MODE, ST_OFFLINE_WAIT };

    void _tryConnect();
    void _enterAPMode();
    bool _connectWiFi();
    void _syncNTP();

    const char* _thingName;
    const char* _mqttHost;
    int         _mqttPort;
    int8_t      _timeZone;
    uint8_t     _dst;

    BearSSL::X509List   _ca;
    BearSSL::X509List   _crt;
    BearSSL::PrivateKey _key;

    WiFiClientSecure _net;
    PubSubClient     _client;

    WiFiCredentialsManager _credManager;
    AccessPointManager     _apManager;
    WiFiCredentials        _creds;

    MqttCallback  _callback;
    State         _state;
    unsigned long _stateStart;

    static const unsigned long OFFLINE_RETRY_MS = 30000UL;
};

#endif
