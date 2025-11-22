#include "ButtonHandler.h"

std::map<uint8_t, ButtonHandler*> ButtonHandler::instances;

void ButtonHandler::setup()
{
    pinMode(pin, INPUT_PULLUP);
    instances[pin] = this;
    attachInterruptArg(digitalPinToInterrupt(pin), 
            reinterpret_cast<void (*)(void*)>(handleInterrupt),
            reinterpret_cast<void*>(static_cast<uint32_t>(pin)),
            FALLING);
}
