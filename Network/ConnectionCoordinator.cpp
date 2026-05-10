#include "ConnectionCoordinator.h"

ConnectionCoordinator::ConnectionCoordinator(WiFiClientSecure& net,
                                             const char* thingName,
                                             const char* mqttHost,
                                             int mqttPort,
                                             int8_t timeZone,
                                             uint8_t dst)
    : _netManager(net, thingName, timeZone, dst)
    , _mqttHost(mqttHost)
    , _mqttPort(mqttPort)
    , _state(ST_BOOT)
    , _stateStart(0)
{
}

void ConnectionCoordinator::begin(MqttCallback mqttCallback, IdleCallback onIdle) {
    _onIdle = onIdle;
    _credManager.begin();
    _netManager.configureMQTT(_mqttHost, _mqttPort, mqttCallback);

    _apManager.onCredentialsSaved([this](WiFiCredentials creds) {
        _creds = creds;
        _netManager.configureWiFi(creds);
        _state = ST_TRY_CONNECT;
    });

    _state = ST_BOOT;
    _stateStart = millis();
}

void ConnectionCoordinator::tick() {
    switch (_state) {
        case ST_BOOT:
            _credManager.readCredentials(_creds);
            if (!_creds.isEmpty()) {
                _netManager.configureWiFi(_creds);
                _state = ST_TRY_CONNECT;
            } else {
                _enterAPMode();
            }
            _stateStart = millis();
            break;

        case ST_TRY_CONNECT:
            _apManager.stop();
            _tryConnect();
            _stateStart = millis();
            break;

        case ST_CONNECTED:
            if (!_netManager.loop()) {
                Serial.println("MQTT disconnected. Reconnecting...");
                _state = ST_TRY_CONNECT;
                _stateStart = millis();
            }
            break;

        case ST_AP_MODE:
            if (_apManager.hasTimedOut()) {
                _state = ST_TRY_CONNECT;
            }
            break;

        case ST_OFFLINE_WAIT:
            if (millis() - _stateStart >= OFFLINE_RETRY_MS) {
                _state = ST_TRY_CONNECT;
            }
            break;
    }
}

void ConnectionCoordinator::_tryConnect() {
    if (!_netManager.connectToWiFi()) {
        if (_credManager.init()) {
            _state = ST_OFFLINE_WAIT;
        } else {
            _enterAPMode();
        }
        return;
    }

    _credManager.saveCredentials(_creds);
    _netManager.syncTimeWithNTP();

    if (!_netManager.connectToMQTT()) {
        Serial.println("MQTT connect failed. Retrying in 30s...");
        _state = ST_OFFLINE_WAIT;
        return;
    }

    _state = ST_CONNECTED;
}

void ConnectionCoordinator::_enterAPMode() {
    if (_apManager.start()) {
        Serial.println(_apManager.getIPAddress().toString().c_str());
    }
    _state = ST_AP_MODE;
    _stateStart = millis();
}

bool ConnectionCoordinator::publish(const char* topic, const char* payload) {
    return _netManager.publish(topic, payload);
}
