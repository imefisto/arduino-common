#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <Arduino.h>

class ButtonHandler {
    public:
        using Callback = std::function<void(void)>;

        ButtonHandler(uint8_t pin, Callback callback, unsigned long debounceTime = 3000UL)
            : pin(pin), callback(callback), debounceTime(debounceTime) {}

        void setup();

    private:
        static void ICACHE_RAM_ATTR handleInterrupt(void* arg) {
            uint8_t isrPin = static_cast<uint8_t>(reinterpret_cast<uint32_t>(arg));
            auto instance = instances[isrPin];
            if (!instance) return;

            unsigned long now = millis();
            if (now - instance->lastInterruptTime > instance->debounceTime) {
                instance->callback();
                instance->lastInterruptTime = now;
            }
        }

        uint8_t pin;
        Callback callback;
        unsigned long debounceTime;
        unsigned long lastInterruptTime = 0;

        static std::map<uint8_t, ButtonHandler*> instances;
};

#endif
