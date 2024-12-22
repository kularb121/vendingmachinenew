#ifndef COINACCEPTOR_H
#define COINACCEPTOR_H

#include <Arduino.h>
#include <EEPROM.h>

class CoinAcceptor {
private:
    int pinCoin;

    unsigned long lastInterruptTime;
    const unsigned long debounceDelay = 20;
    bool state;
    int coinDiscard = 2; // New variable to hold the number of coins to discard
    unsigned long lastCoinTime; // Timestamp of the last coin insertion
    
    // EEPROM registers
    // register blocks 201 - 220 are reserved for CoinAcceptor
    int regCoinCount = 201; // New variable to hold the EEPROM address for count

public:
    // Constructor
    volatile int count;
    CoinAcceptor(int coinPin, unsigned long debounceDelay = 20) 
        : pinCoin(coinPin), count(0), lastInterruptTime(0), debounceDelay(debounceDelay), state(false) {
        pinMode(pinCoin, INPUT);
        EEPROM.begin(512); // Initialize EEPROM with size 512 bytes
        if(EEPROM.read(regCoinCount) == 255) { // Check if the EEPROM value is empty
            EEPROM.write(regCoinCount, 0); // Initialize the count value in EEPROM
            EEPROM.commit(); // Ensure the value is written to EEPROM
        }
        Serial.println("Coin now is " + String(count));
        count = EEPROM.read(regCoinCount); // Read the count value from EEPROM
    }

    // Getter and Setter for pinCoin
    int getPinCoin() const {
        return pinCoin;
    }

    void setPinCoin(int newPinCoin) {
        pinCoin = newPinCoin;
        pinMode(pinCoin, INPUT);
    }

    void initCount() {
        count = EEPROM.read(regCoinCount); // Read the count value from EEPROM
    }

    // Getter and Setter for count
    int getCount() const {
        return count;
    }

    void setCount(int newCount) {
        count = newCount;
        EEPROM.write(regCoinCount, count); // Write the count value to EEPROM
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
        EEPROM.write(regCoinCount, 0); // Write the count value to EEPROM
    }

    // Method to manually increment the count (e.g., called from loop)
    void incrementCount() {
       
        unsigned long currentTime = millis();
        if (currentTime - lastInterruptTime > debounceDelay) {
            count++;
            lastInterruptTime = currentTime;

        }
    }
        // Method to check and reset the count if more than five minutes have passed
    int checkAndResetCount() {

        unsigned long currentTime = millis();
        if (currentTime - lastCoinTime > 300000) { // 300000 ms = 5 minutes
            int coinReset = getCount();
            resetCount();
            lastCoinTime = millis();
            return coinReset;
        }
        else {
            return 0;
        }
    }
};

#endif // COINACCEPTOR_H