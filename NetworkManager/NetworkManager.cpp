#include "NetworkManager.h"
#include "secrets.h"

NetworkManager::NetworkManager(const char* ssid, const char* password, const char* thingName, int8_t timeZone, uint8_t dst)
    : ssid(ssid), password(password), thingName(thingName), client(net), timeZone(timeZone), dst(dst) {
}

void NetworkManager::connectToWiFi() {
    Serial.print("Connecting to Wi-Fi");
#ifdef ESP32
    WiFi.setHostname(thingName.c_str());
#else
    WiFi.hostname(thingName.c_str());
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
}

void NetworkManager::connectToMqtt() {
    if (!client.connected()) {
        Serial.print("Connecting to MQTT... ");
        while (!client.connected()) {
            if (client.connect(thingName.c_str())) {
                Serial.println("Connected!");
                std::string topic = thingName + "/sub";
                client.subscribe(topic.c_str());
            } else {
                Serial.print("Failed, rc=");
                Serial.print(client.state());
                Serial.println(" Retrying in 5 seconds...");
                delay(5000);
            }
        }
    }
}

void NetworkManager::connectToNTP() {
    configTime(timeZone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println();
}

void NetworkManager::setup() {
    connectToWiFi();
    connectToNTP();
    
    BearSSL::X509List cert(AWS_CERT_CA);
    BearSSL::X509List client_crt(AWS_CERT_CRT);
    BearSSL::PrivateKey key(AWS_CERT_PRIVATE);

#ifdef ESP32
    net.setCACert(cert);
    net.setCertificate(client_crt);
    net.setPrivateKey(key);
#else
    net.setTrustAnchors(&cert);
    net.setClientRSACert(&client_crt, &key);
#endif

    client.setServer(MQTT_HOST, MQTT_PORT);
    connectToMqtt();
}

void NetworkManager::loop() {
    if (!client.connected()) {
        connectToMqtt();
    }
    client.loop();
}

bool NetworkManager::publish(const char* topic, const char* payload) {
    return client.publish(topic, payload);
}

void NetworkManager::setCallback(void (*callback)(char*, byte*, unsigned int)) {
    client.setCallback(callback);
}
