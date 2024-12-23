#include "VendingMachine.h"
#include "Mechanics.h"
#include <Arduino.h>
#include <esp_task_wdt.h>

VendingMachine::VendingMachine(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin, int regPumpTime) {
    setPinLed(ledPin);
    setPinButton(buttonPin);
    setPinButtonConfigure(buttonConfigurePin);
    setPinPump(pumpPin);
    buttonPressed = false;
    buttonConfigurePressed = false;
    startPrice = 5.0; // Initialize startPrice

    // Read timePerBaht from EEPROM
    timePerBaht = EEPROM.read(regPumpTime);
    if (timePerBaht == 0 || timePerBaht == 255) {
        timePerBaht = 16; // Set default value, in 100ms
        EEPROM.write(regPumpTime, timePerBaht);
        EEPROM.commit();
    }
}

int VendingMachine::handleButtonPress(int pinInput, int pinOutput, volatile int &coinCount) 
{
    // Determine which button is being pressed and use the appropriate debounce timing variable
    if (pinInput == pinButton) 
    {
        if (digitalRead(pinInput) == HIGH) {
            delay(100);
            if (digitalRead(pinInput) == HIGH) {
                delay(100);
                if(digitalRead(pinInput) == HIGH) {
                    Serial.println("Button pressed");
                    int tempCount = coinCount;
                    checkAndTriggerOperation(coinCount);
                    return tempCount;
                }
            }
        }
    }
    

    return 0;
}

int VendingMachine::handleButtonConfigurePress()
{
    if(blinkState == false)
    {
        if (digitalRead(pinButtonConfigure) == HIGH) {
            delay(100);
            if (digitalRead(pinButtonConfigure) == HIGH) {
                delay(100);
                if(digitalRead(pinButtonConfigure) == HIGH) {
                    Serial.println("pinButtonConfigure pressed");
                    long currentTime = millis();
                    if(digitalRead(pinButtonConfigure) == HIGH)
                        blinkState = true;
                }
            }
        }
    }

    else
    {
        currentBlinkMillis = millis();
        if(currentBlinkMillis - previousBlinkMillis >= 200)
        {
            previousBlinkMillis = currentBlinkMillis;
            digitalWrite(pinLed, !digitalRead(pinLed));
        }
        if (digitalRead(pinButtonConfigure) == HIGH) {
            delay(100);
            if (digitalRead(pinButtonConfigure) == HIGH) {
                delay(100);
                if(digitalRead(pinButtonConfigure) == HIGH) 
                {
                    long press_StartTime = millis();
                    long press_StopTime;
                    while (digitalRead(pinButtonConfigure) == HIGH) {
                        if (millis() - press_StartTime > 15000) {
                            break;
                        }
                    }
                    press_StopTime = millis();
                    int press_Duration = (press_StopTime - press_StartTime) / 100;
                    Serial.println("Press duration: " + String(press_Duration));
                    blinkState = false;
                    digitalWrite(pinLed, LOW);

    //                     unsigned long timePerBaht;

    // // EEPROM registers 310 - 330 are reserved for VendingMachine
    // int regPumpTime; // EEPROM address for timePerBaht
                    return 3;
                }
            }

    }

        return 0;
    }
}

int VendingMachine::handleAllButtonPresses(volatile int &coinCount) 
{
    setLedState(coinCount);
    if(digitalRead(pinButton) == HIGH)
    {
        delay(20);
        if(digitalRead(pinButton) == HIGH)
        {
            return handleButtonPress(pinButton, pinLed, coinCount);
        }
    }

    return 0;
}

void VendingMachine::checkAndTriggerOperation(volatile int &coinCount) {
    if (coinCount >= startPrice) {
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