#include "VendingMachine.h"
#include <Arduino.h>

VendingMachine::VendingMachine(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin) {
    setPinLed(ledPin);
    setPinButton(buttonPin);
    setPinButtonConfigure(buttonConfigurePin);
    setPinPump(pumpPin);
}

void VendingMachine::checkButtonAndBlink(int pinInput, int pinOutput) 
{
    const int debounceDelay = 50; // milliseconds
    bool reading = digitalRead(pinInput);
    if (reading) 
    {
        delay(debounceDelay);
        reading = digitalRead(pinInput);
        if(reading)
        {
            Serial.println("Button pressed, blinking LED");
            for (int i = 0; i < 5; ++i) {
                digitalWrite(pinOutput, HIGH);
                delay(500);
                digitalWrite(pinOutput, LOW);
                delay(500);
            }
        }
    }
}



// Setter functions
void VendingMachine::setPinLed(int ledPin) {
    pinLed = ledPin;
    pinMode(pinLed, OUTPUT);
}

void VendingMachine::setPinButton(int buttonPin) {
    pinButton = buttonPin;
    pinMode(pinButton, INPUT);
}

void VendingMachine::setPinButtonConfigure(int buttonConfigurePin) {
    pinButtonConfigure = buttonConfigurePin;
    pinMode(pinButtonConfigure, INPUT);
}

void VendingMachine::setPinPump(int pumpPin) {
    pinPump = pumpPin;
    pinMode(pinPump, OUTPUT);
}

// Getter functions
int VendingMachine::getPinLed() const {
    return pinLed;
}

int VendingMachine::getPinButton() const {
    return pinButton;
}

int VendingMachine::getPinButtonConfigure() const {
    return pinButtonConfigure;
}

int VendingMachine::getPinPump() const {
    return pinPump;
}