#include "NetworkManager.h"

NetworkManager::NetworkManager(WiFiClientSecure& net, const char* thingName, int8_t timeZone, uint8_t dst)
    : wiFiClient(net),
    client(wiFiClient),
    thingName(thingName),
    timeZone(timeZone),
    dst(dst)
{
}

void NetworkManager::configureWiFi(WiFiCredentials creds)
{
    ssid = creds.ssid;
    password = creds.password;
}

bool NetworkManager::connectToWiFi() {
#ifdef ESP32
    WiFi.setHostname(thingName.c_str());
#else
    WiFi.hostname(thingName.c_str());
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::syncTimeWithNTP() {
    configTime(timeZone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        now = time(nullptr);
    }
}

void NetworkManager::configureMQTT(const char *mqttHost, int mqttPort, ClientCallback callback) {
    client.setServer(mqttHost, mqttPort);
    client.setCallback(callback);
}

bool NetworkManager::connectToMQTT() {
    if (client.connected()) {
        return true;
    }

    if (client.connect(thingName.c_str())) {
        std::string topic = thingName + "/sub";
        client.subscribe(topic.c_str());
        return true;
    }

    return false;
}

bool NetworkManager::loop() {
    if (!connectToMQTT()) {
        return false;
    }

    client.loop();
    return true;
}

bool NetworkManager::publish(const char* topic, const char* payload) {
    return client.publish(topic, payload);
}
