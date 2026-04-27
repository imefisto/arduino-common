#ifndef CONNECTION_COORDINATOR_H
#define CONNECTION_COORDINATOR_H

#include <functional>
#include <WiFiClientSecure.h>
#include "NetworkManager.h"
#include "AccessPointManager.h"
#include "WiFiCredentialsManager.h"

class ConnectionCoordinator {
public:
    using MqttCallback = std::function<void(char*, byte*, unsigned int)>;
    using IdleCallback = std::function<void()>;

    ConnectionCoordinator(WiFiClientSecure& net,
                          const char* thingName,
                          const char* mqttHost,
                          int mqttPort,
                          int8_t timeZone,
                          uint8_t dst);

    void begin(MqttCallback mqttCallback, IdleCallback onIdle = nullptr);
    void tick();

    bool isConnected() { return _netManager.isConnected(); }
    bool publish(const char* topic, const char* payload);

    WiFiCredentialsManager& credManager() { return _credManager; }

private:
    enum State { ST_BOOT, ST_TRY_CONNECT, ST_CONNECTED, ST_AP_MODE, ST_OFFLINE_WAIT };

    void _tryConnect();
    void _enterAPMode();

    NetworkManager _netManager;
    AccessPointManager _apManager;
    WiFiCredentialsManager _credManager;

    const char* _mqttHost;
    int _mqttPort;

    WiFiCredentials _creds;
    State _state;
    unsigned long _stateStart;
    IdleCallback _onIdle;

    static const unsigned long OFFLINE_RETRY_MS    = 30000UL;
    static const unsigned long MQTT_RETRY_DELAY_MS = 5000UL;
};

#endif
