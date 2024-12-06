#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

#include "Mechanics.h"
#include <EEPROM.h>

class VendingMachine {
public:
    VendingMachine(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin);
    
    void checkButtonAndBlink(int pinInput, int pinOutput);
    int handleButtonPress(int pinInput, int pinOutput, volatile int &coinCount);
    int handleAllButtonPresses(volatile int &coinCount);
    void checkAndTriggerOperation(volatile int &coinCount);
    void setLedState(volatile int &coinCount);

    // Setter functions
    void setPinLed(int ledPin);
    void setPinButton(int buttonPin);
    void setPinButtonConfigure(int buttonConfigurePin);
    void setPinPump(int pumpPin);
    void setButtonPressed(bool state);
    void setButtonConfigurePressed(bool state);
    void setStartPrice(float price);
    void setTimePerBaht(unsigned long time);

    // Getter functions
    int getPinLed() const;
    int getPinButton() const;
    int getPinButtonConfigure() const;
    int getPinPump() const;
    bool isButtonPressed() const;
    bool isButtonConfigurePressed() const;
    float getStartPrice() const;
    unsigned long getTimePerBaht() const;

private:
    int pinLed;
    int pinButton;
    int pinButtonConfigure;
    int pinPump;
    volatile bool buttonPressed;
    volatile bool buttonConfigurePressed;
    const unsigned long debounceDelay = 50; // Debounce delay in milliseconds
    volatile unsigned long lastButtonPressTime = 0;    
    volatile unsigned long lastButtonConfigurePressTime = 0;
    float startPrice; // New variable to hold the start price
    unsigned long timePerBaht;
    const int timePerBahtAddress = 311; // EEPROM address for timePerBaht



};

#endif // VENDINGMACHINE_H