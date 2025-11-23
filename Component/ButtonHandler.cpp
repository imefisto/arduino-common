#include "ButtonHandler.h"

void ButtonHandler::setup()
{
    pinMode(pin, INPUT_PULLUP);
    attachInterruptArg(digitalPinToInterrupt(pin), classIsr, this, FALLING);
};

void ButtonHandler::process()
{
    if (buttonPressed) {
        buttonPressed = false;
        handleAction();
    }
}
