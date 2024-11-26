#ifndef COINACCEPTOR_H
#define COINACCEPTOR_H

#include <Arduino.h>
#include <EEPROM.h>

class CoinAcceptor {
private:
    int pinCoin;

    unsigned long lastInterruptTime;
    const unsigned long debounceDelay = 50;
    bool state;
    int coinDiscard = 2; // New variable to hold the number of coins to discard
    int regCount; // New variable to hold the EEPROM address for count
    unsigned long lastCoinTime; // Timestamp of the last coin insertion
    
public:
    // Constructor
    volatile int count;
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
        //     // EEPROM.write(regCount, count); // Write the count value to EEPROM
        //     // EEPROM.commit(); // Ensure the value is written to EEPROM
        }
    }

        // Method to check and reset the count if more than five minutes have passed
    void checkAndResetCount() {
        unsigned long currentTime = millis();
        if (currentTime - lastCoinTime > 300000) { // 300000 ms = 5 minutes
            resetCount();
        }
    }
};

#endif // COINACCEPTOR_H