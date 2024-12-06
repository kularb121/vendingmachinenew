#include "VendingMachine.h"
#include "Mechanics.h"
#include <Arduino.h>
#include <esp_task_wdt.h>

VendingMachine::VendingMachine(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin) {
    setPinLed(ledPin);
    setPinButton(buttonPin);
    setPinButtonConfigure(buttonConfigurePin);
    setPinPump(pumpPin);
    buttonPressed = false;
    buttonConfigurePressed = false;
    startPrice = 5.0; // Initialize startPrice

    // Read timePerBaht from EEPROM
    timePerBaht = EEPROM.read(timePerBahtAddress);
    if (timePerBaht == 0 || timePerBaht == 255) {
        timePerBaht = 16; // Set default value, in 100ms
        EEPROM.write(timePerBahtAddress, timePerBaht);
        EEPROM.commit();
    }
}

int VendingMachine::handleButtonPress(volatile bool &buttonPressed, int pinInput, int pinOutput, volatile int &coinCount) {
    unsigned long currentTime = millis();
    volatile unsigned long *lastPressTime;

    // Determine which button is being pressed and use the appropriate debounce timing variable
    if (pinInput == pinButton) {
        lastPressTime = &lastButtonPressTime;
    } else if (pinInput == pinButtonConfigure) {
        lastPressTime = &lastButtonConfigurePressTime;
    } else {
        // Handle other buttons if necessary
        return 0;
    }

    if (buttonPressed) {
        if (currentTime - *lastPressTime > debounceDelay * 4) {
            Serial.println("Button pressed");
            if (pinInput == pinButton) {
                int tempCount = coinCount;
                checkAndTriggerOperation(coinCount);
                return tempCount;
            }
            buttonPressed = false;
            *lastPressTime = currentTime; // Update the last button press time
            return 0;
        }
        return 0;
    }
    else 
        return 0;
}

int VendingMachine::handleAllButtonPresses(volatile int &coinCount) 
{
    setLedState(coinCount);
    if (buttonPressed) {
        return handleButtonPress(buttonPressed, pinButton, pinLed, coinCount);
    }

    if (buttonConfigurePressed) {
        return handleButtonPress(buttonConfigurePressed, pinButtonConfigure, pinLed, coinCount);
    }

    return 0;
}

void VendingMachine::checkAndTriggerOperation(volatile int &coinCount) {
    if (coinCount >= startPrice) {
        if (buttonPressed) {
            unsigned long operationTime = (timePerBaht * 100) * coinCount;
            unsigned long startTime = millis();
            digitalWrite(pinPump, HIGH); // Trigger the pump

            while (millis() - startTime < operationTime) {
                digitalWrite(pinLed, HIGH);
                delay(100); // Short delay of 100 ms
                digitalWrite(pinLed, LOW);
                delay(100); // Short delay of 100 ms
                esp_task_wdt_reset(); // Reset the watchdog timer
            }

            digitalWrite(pinPump, LOW); // Turn off the pump
            buttonPressed = false; // Reset the button press state
            coinCount = 0; // Reset the coin count
        }
    }
}

void VendingMachine::setLedState(volatile int &coinCount) {
    if (coinCount >= startPrice) {
        digitalWrite(pinLed, HIGH);
    }
    else
        digitalWrite(pinLed, LOW);    
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
    if (currentTime - lastButtonPressTime > debounceDelay * 2) {
        buttonPressed = state;
        lastButtonPressTime = currentTime;
    }
}

void VendingMachine::setButtonConfigurePressed(bool state) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastButtonConfigurePressTime > debounceDelay * 2) {
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