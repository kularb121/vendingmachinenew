#include "VendingMachine.h"
#include "Mechanics.h"
#include <Arduino.h>

VendingMachine::VendingMachine(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin) {
    setPinLed(ledPin);
    setPinButton(buttonPin);
    setPinButtonConfigure(buttonConfigurePin);
    setPinPump(pumpPin);
    buttonPressed = false;
    buttonConfigurePressed = false;
    startPrice = 0.0; // Initialize startPrice

    // Read timePerBaht from EEPROM
    timePerBaht = EEPROM.read(timePerBahtAddress);
    if (timePerBaht == 0 || timePerBaht == 255) {
        timePerBaht = 16; // Set default value
        EEPROM.write(timePerBahtAddress, timePerBaht);
        EEPROM.commit();
    }
}

void VendingMachine::handleButtonPress(volatile bool &buttonPressed, int pinInput, int pinOutput, volatile int &coinCount) {
    if (buttonPressed) {
        if(pinInput == pinButton)
            checkAndTriggerOperation(coinCount);
        //checkButtonAndBlink(pinButton, pinLed);
        buttonPressed = false;
    }
}

void VendingMachine::handleAllButtonPresses(volatile int &coinCount) 
{
    if (buttonPressed) {
        handleButtonPress(buttonPressed, pinButton, pinLed, coinCount);
    }

    if (buttonConfigurePressed) {
        handleButtonPress(buttonConfigurePressed, pinButtonConfigure, pinLed, coinCount);
    }
}

void VendingMachine::checkAndTriggerOperation(volatile int &coinCount) {
    if (coinCount >= startPrice) {
        if (buttonPressed) {
            unsigned long operationTime = timePerBaht * coinCount;
            digitalWrite(pinPump, HIGH); // Trigger the pump
            delay(operationTime); // Keep the pump on for the calculated time
            digitalWrite(pinPump, LOW); // Turn off the pump
            buttonPressed = false; // Reset the button press state
            coinCount = 0; // Reset the coin count
        }
    }
}

void VendingMachine::checkButtonAndBlink(int pinInput, int pinOutput) {
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

void VendingMachine::setButtonPressed(bool state) {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonPressTime > debounceDelay) {
        buttonPressed = state;
        lastButtonPressTime = currentTime;
    }
}

void VendingMachine::setButtonConfigurePressed(bool state) {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonConfigurePressTime > debounceDelay) {
        buttonConfigurePressed = state;
        lastButtonConfigurePressTime = currentTime;
    }
}

void VendingMachine::setStartPrice(float price) {
    startPrice = price;
}

void VendingMachine::setTimePerBaht(unsigned long time) {
    timePerBaht = time;
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

bool VendingMachine::isButtonPressed() const {
    return buttonPressed;
}

bool VendingMachine::isButtonConfigurePressed() const {
    return buttonConfigurePressed;
}

float VendingMachine::getStartPrice() const {
    return startPrice;
}

unsigned long VendingMachine::getTimePerBaht() const {
    return timePerBaht;
}