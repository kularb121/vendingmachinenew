#ifndef VENDINGMACHINE_H
#define VENDINGMACHINE_H

class VendingMachine {
public:
    VendingMachine(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin);
    
        void checkButtonAndBlink(int pinInput, int pinOutput);

    // Setter functions
    void setPinLed(int ledPin);
    void setPinButton(int buttonPin);
    void setPinButtonConfigure(int buttonConfigurePin);
    void setPinPump(int pumpPin);

    // Getter functions
    int getPinLed() const;
    int getPinButton() const;
    int getPinButtonConfigure() const;
    int getPinPump() const;

    int pinLed;
    int pinButton;
    int pinButtonConfigure;
    int pinPump;
};

#endif // VENDINGMACHINE_H