#ifndef WIFI_CREDENTIALS_MANAGER_H
#define WIFI_CREDENTIALS_MANAGER_H

#include <EEPROM.h>
#include "WiFiCredentials.h"

class WiFiCredentialsManager {
    public:
        WiFiCredentialsManager(size_t ssidAddr = 0, size_t passAddr = 64);
        bool begin(size_t eepromSize = 512);
        bool init() const;
        bool readCredentials(WiFiCredentials &creds);
        bool saveCredentials(WiFiCredentials creds);
        bool clearCredentials();

    private:
        void markInitialized();
        bool hasBegun() const { return _hasBegun; }

        const size_t MAGIC_ADDR = 132;
        const uint16_t MAGIC_NUMBER = 0xABCD;
        const size_t SSID_ADDR;
        const size_t PASS_ADDR;
        bool _hasBegun = false;
        WiFiCredentials cachedCredentials;
};

#endif
