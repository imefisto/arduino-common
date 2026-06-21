#include "NetworkManager.h"
#include <time.h>

NetworkManager::NetworkManager(const char* thingName,
                               const char* mqttHost, int mqttPort,
                               const char* caCert, const char* clientCert, const char* privateKey,
                               int8_t timeZone, uint8_t dst)
    : _thingName(thingName)
    , _mqttHost(mqttHost)
    , _mqttPort(mqttPort)
    , _timeZone(timeZone)
    , _dst(dst)
    , _ca(caCert)
    , _crt(clientCert)
    , _key(privateKey)
    , _client(_net)
    , _state(ST_BOOT)
    , _stateStart(0)
{
}

void NetworkManager::begin(MqttCallback callback) {
    _callback = callback;
    _credManager.begin();
    _client.setServer(_mqttHost, _mqttPort);
    _client.setCallback(callback);
    _client.setKeepAlive(15);

    _apManager.onCredentialsSaved([this](WiFiCredentials creds) {
        _creds = creds;
        _state = ST_TRY_CONNECT;
    });
}

void NetworkManager::loop() {
    switch (_state) {
        case ST_BOOT:
            if (_credManager.readCredentials(_creds)) {
                _state = ST_TRY_CONNECT;
            } else {
                _enterAPMode();
            }
            break;

        case ST_TRY_CONNECT:
            _apManager.stop();
            _tryConnect();
            _stateStart = millis();
            break;

        case ST_CONNECTED:
            if (!_client.connected()) {
                Serial.println("[net] MQTT disconnected, reconnecting...");
                _state = ST_TRY_CONNECT;
            } else {
                _client.loop();
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

void NetworkManager::_tryConnect() {
    if (!_connectWiFi()) {
        if (_credManager.init()) {
            Serial.println("[net] WiFi failed, retrying in 30s...");
            _state = ST_OFFLINE_WAIT;
        } else {
            _enterAPMode();
        }
        return;
    }

    _credManager.saveCredentials(_creds);
    _syncNTP();

    // Reset TLS state and re-apply certs on every attempt.
    // This reinitialises BearSSL after any prior abrupt disconnect.
    _net.stop();
    _net.setTrustAnchors(&_ca);
    _net.setClientRSACert(&_crt, &_key);

    if (_client.connect(_thingName)) {
        _client.subscribe((std::string(_thingName) + "/sub").c_str());
        Serial.println("[net] MQTT connected.");
        _state = ST_CONNECTED;
        return;
    }

    Serial.println("[net] MQTT connect failed, retrying in 30s...");
    _state = ST_OFFLINE_WAIT;
}

bool NetworkManager::_connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return true;

#ifdef ESP32
    WiFi.setHostname(_thingName);
#else
    WiFi.hostname(_thingName);
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(_creds.ssid, _creds.password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::_syncNTP() {
    configTime(_timeZone * 3600, _dst * 3600, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        now = time(nullptr);
    }
}

bool NetworkManager::publish(const char* topic, const char* payload) {
    return _client.publish(topic, payload);
}

void NetworkManager::_enterAPMode() {
    if (_apManager.start()) {
        Serial.print("[net] AP mode, IP: ");
        Serial.println(_apManager.getIPAddress().toString().c_str());
    }
    _state = ST_AP_MODE;
}
