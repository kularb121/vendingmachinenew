#include "Mechanics.h"

Mechanics::Mechanics(uint8_t lcdAddr, uint8_t lcdCols, uint8_t lcdRows)
    : lcd(lcdAddr, lcdCols, lcdRows) {}

void Mechanics::init() {
    delay(1000);
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    pinMode(pinRouter, OUTPUT);
}

// void Mechanics::updateCoinDisplay(int count, bool firstTime) {
//     unsigned long currentMillis = millis();
//     if (currentMillis - previousMillis >= interval) {
//         previousMillis = currentMillis;
//         lcd.clear();
//         lcd.setCursor(0, 0);
//         if (!firstTime) {
//             if (count >= 0) {
//                 lcd.print("Coins: ");
//                 lcd.print(count);
//             } else {
//                 lcd.print("Coins: Invalid");
//                 delay(5000);
//                 ESP.restart();
//             }
//         } else {
//             lcd.print("Welcome to");
//             lcd.setCursor(0, 1);
//             lcd.print("Wash Connect");
//             delay(2000);
//         }
//     }
// }

void Mechanics::updateCoinDisplay(int count, bool firstTime) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        if (firstTime || count != lastDisplayedCount) {
            lcd.clear();
            lcd.setCursor(0, 0);
            if (!firstTime) {
                if (count >= 0) {
                    lcd.print("Coins: ");
                    lcd.print(count);
                    lastDisplayedCount = count; // Update the last displayed count
                } else {
                    lcd.print("Coins: Invalid");
                    delay(5000);
                    ESP.restart();
                }
            } else {
                lcd.print("Welcome to");
                lcd.setCursor(0, 1);
                lcd.print("Wash Connect");
                delay(2000);
            }
        }
    }
}

void Mechanics::updateDisplay(int col1, int row1, String message1, int col2, int row2, String message2,
                              int col3, int row3, String message3, int col4, int row4, String message4, int displayTime) 
{
    lcd.clear();
    if (message1 != "-") {
        lcd.setCursor(col1, row1);
        lcd.print(message1);
    }
    if (message2 != "-") {
        lcd.setCursor(col2, row2);
        lcd.print(message2);
    }
    if (message3 != "-") {
        lcd.setCursor(col3, row3);
        lcd.print(message3);
    }
    if (message4 != "-") {
        lcd.setCursor(col4, row4);
        lcd.print(message4);
    }
    delay(displayTime * 1000);
}