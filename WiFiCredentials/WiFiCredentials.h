#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

#include <cstring>

struct WiFiCredentials {
    char ssid[32];
    char password[64];

    bool operator==(const WiFiCredentials& other) const {
        return (strcmp(ssid, other.ssid) == 0 &&
                strcmp(password, other.password) == 0);
    }

    WiFiCredentials() = default;

    WiFiCredentials(const char* ssid, const char* password)
    {
        strcpy(this->ssid, ssid);
        strcpy(this->password, password);
    }

    void reset() {
        memset(ssid, 0xFF, sizeof(ssid));
        memset(password, 0xFF, sizeof(password));
    }

    bool isEmpty() const {
        return ssid[0] == '\0' || ssid[0] == 0xFF;
    }
};

#endif
