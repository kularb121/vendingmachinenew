#ifndef MECHANICS_H
#define MECHANICS_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Mechanics {
public:
    Mechanics(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows);
    void init();
    void updateCoinDisplay(int count, bool firstTime);
    void updateDisplay(int col1, int row1, String message1, int col2, int row2, String message2,
                       int col3, int row3, String message3, int col4, int row4, String message4, int displayTime);

private:
    LiquidCrystal_I2C lcd;
    unsigned long previousMillis = 0;
    const long interval = 1000; // Interval to update the display
    int pinRouter = 14; // Pin to control the router
    int lastDisplayedCount = -1; // Variable to store the last displayed count
};

#endif // MECHANICS_H

