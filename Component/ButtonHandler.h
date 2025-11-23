#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <Arduino.h>

class ButtonHandler {
    public:
        ButtonHandler(uint8_t p, unsigned long debounceMs = 250) 
            : pin(p), debounceTime(debounceMs), lastInterruptTime(0), buttonPressed(false) {}

        void setup();
        void process();

    protected:
        volatile bool buttonPressed;
        virtual void handleAction() {}

    private:
        IRAM_ATTR static void classIsr(void *p) {
            ButtonHandler *ptr = (ButtonHandler*) p;
            ptr->instanceIsr();
        }
    
        IRAM_ATTR virtual void instanceIsr() {
            unsigned long now = millis();
            if (now - lastInterruptTime > debounceTime) {
                handleButtonPress();
            }

            lastInterruptTime = now;
        }

        IRAM_ATTR virtual void handleButtonPress() {
            buttonPressed = true;
        }
    
        uint8_t pin;
        const unsigned long debounceTime;
        unsigned long lastInterruptTime;
};

#endif
