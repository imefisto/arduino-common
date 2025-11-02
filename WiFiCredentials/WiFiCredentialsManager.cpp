#include "Arduino.h"
#include "WiFiCredentialsManager.h"

WiFiCredentialsManager::WiFiCredentialsManager(size_t ssidAddr, size_t passAddr)
    : SSID_ADDR(ssidAddr), PASS_ADDR(passAddr) {
    }

bool WiFiCredentialsManager::begin(size_t eepromSize) {
    if (!_hasBegun) {
        EEPROM.begin(eepromSize);
        _hasBegun = true;
    }
    
    return true;
}

bool WiFiCredentialsManager::init() const {
    uint16_t magic;
    EEPROM.get(MAGIC_ADDR, magic);
    return (magic == MAGIC_NUMBER);
}

void WiFiCredentialsManager::markInitialized() {
    if (!hasBegun()) return;
    
    EEPROM.put(MAGIC_ADDR, MAGIC_NUMBER);
    EEPROM.commit();
}

bool WiFiCredentialsManager::readCredentials(WiFiCredentials &creds) {
    if (!hasBegun()) {
        return false;
    }

    if (!init()) {
        return false;
    }

    EEPROM.get(SSID_ADDR, creds.ssid);
    EEPROM.get(PASS_ADDR, creds.password);
    
    cachedCredentials = creds;

    return !creds.isEmpty();
}

bool WiFiCredentialsManager::saveCredentials(WiFiCredentials creds) {
    if (!hasBegun()) {
        return false;
    }

    if (cachedCredentials == creds) {
        return true;
    }
    
    clearCredentials();

    EEPROM.put(SSID_ADDR, creds.ssid);
    EEPROM.put(PASS_ADDR, creds.password);
    
    markInitialized();
    
    cachedCredentials = creds;
    EEPROM.commit();

    return true;
}

bool WiFiCredentialsManager::clearCredentials() {
    if (!hasBegun()) return false;

    for (size_t i = 0; i < sizeof(WiFiCredentials::ssid); i++) {
        EEPROM.write(SSID_ADDR + i, 0xFF);
    }
    for (size_t i = 0; i < sizeof(WiFiCredentials::password); i++) {
        EEPROM.write(PASS_ADDR + i, 0xFF);
    }

    // Clear initialization marker
    EEPROM.put(MAGIC_ADDR, (uint16_t)0xFFFF);
    EEPROM.commit();

    cachedCredentials.reset();

    return true;
}
