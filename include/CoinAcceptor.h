#ifndef COINACCEPTOR_H
#define COINACCEPTOR_H

#include <Arduino.h>
#include <EEPROM.h>

class CoinAcceptor {
private:
    int pinCoin;
    volatile int count;
    unsigned long lastInterruptTime;
    const unsigned long debounceDelay;
    bool state;
    int regCount; // New variable to hold the EEPROM address for count

public:
    // Constructor
    CoinAcceptor(int coinPin, unsigned long debounceDelay = 20, int regCount = 221) 
        : pinCoin(coinPin), count(0), lastInterruptTime(0), debounceDelay(debounceDelay), state(false) {
        pinMode(pinCoin, INPUT);
        EEPROM.begin(512); // Initialize EEPROM with size 512 bytes
        if(EEPROM.read(regCount) == 255) { // Check if the EEPROM value is empty
            EEPROM.write(regCount, 0); // Initialize the count value in EEPROM
            EEPROM.commit(); // Ensure the value is written to EEPROM
        }
        count = EEPROM.read(regCount); // Read the count value from EEPROM
    }

    // Getter and Setter for pinCoin
    int getPinCoin() const {
        return pinCoin;
    }

    void setPinCoin(int newPinCoin) {
        pinCoin = newPinCoin;
        pinMode(pinCoin, INPUT);
    }

    // Getter and Setter for count
    int getCount() const {
        return count;
    }

    void setCount(int newCount) {
        count = newCount;
        EEPROM.write(regCount, count); // Write the count value to EEPROM
        EEPROM.commit(); // Ensure the value is written to EEPROM
    }

    // Getter and Setter for state
    bool getState() const {
        return state;
    }

    void setState(bool newState) {
        state = newState;
    }

    // Reset count
    void resetCount() {
        setCount(0);
    }

    // Method to manually increment the count (e.g., called from loop)
    void incrementCount() {
        unsigned long currentTime = millis();
        if (currentTime - lastInterruptTime > debounceDelay) {
            count++;
            lastInterruptTime = currentTime;
            EEPROM.write(regCount, count); // Write the count value to EEPROM
            EEPROM.commit(); // Ensure the value is written to EEPROM
        }
    }
};

#endif // COINACCEPTOR_H