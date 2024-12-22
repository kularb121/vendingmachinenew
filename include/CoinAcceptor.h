#ifndef COINACCEPTOR_H
#define COINACCEPTOR_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Mechanics.h"

class CoinAcceptor {
private:
    int pinCoin;
    unsigned long lastInterruptTime;
    const unsigned long debounceDelay = 20;
    bool state;
    int coinDiscard = 2; // New variable to hold the number of coins to discard
   
    // EEPROM registers
    // register blocks 201 - 220 are reserved for CoinAcceptor
    int regCoinCount = 201; // New variable to hold the EEPROM address for count

public:
    volatile bool coinInsertedFlag = false;
    unsigned long lastCoinTime; // Timestamp of the last coin insertion
    // Constructor
    volatile int count;
    volatile unsigned long lastCoinInsertedTime = 0;
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
    // Method to count coins
    void countCoins(volatile int &pulseCount) 
    {
        // Adjust count (divide by 2 if >= 2, otherwise reset)
        int coin10 = 10;
        int coin5 = 5;
        int leadCoin = 2;

        if (count >= leadCoin) {
            Serial.println("countCoins is called.");
            if (pulseCount >= coin10) {
                count = count + round(pulseCount / (coin10 + leadCoin)) * coin10;
            } 
            else if (pulseCount >= coin5) {
                count = count + round(pulseCount / (coin5 + leadCoin)) * coin5;
            } 
            Serial.println("count is " + String(count));           
        }
        
    }
    // Method to handle coin insertion
    void handleCoinInsertion(Mechanics &mechanics, volatile int &pulseCount) 
    {
        if (coinInsertedFlag) {
            unsigned long currentTime = millis();
            if (currentTime - lastCoinTime >= 750) {
                Serial.println("Coin to calculate is "+ String(pulseCount));
                countCoins(pulseCount);
                mechanics.updateCoinDisplay(getCount(), false, true);
                lastCoinTime = currentTime;
                coinInsertedFlag = false;
                pulseCount = 0;
            }
        }
        else 
            mechanics.updateCoinDisplay(getCount(), false, true);
    }
};
#endif // COINACCEPTOR_H