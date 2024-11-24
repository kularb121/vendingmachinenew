#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "VendingMachine.h"

// Initialize VendingMachine with pin numbers
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
VendingMachine vm_detergent(25, 33, 36, 12); //(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin)
VendingMachine vm_softener(32, 39, 34, 13);


void updateCoinDisplay(int count, bool firstTime) {
    lcd.clear();
    lcd.setCursor(0, 0);
    if(!firstTime) {
        if(count > 0) {
            lcd.print("Coins: ");
            lcd.print(count);
        }
        else 
        {
            lcd.print("Coins: Invalid");
            delay(5000);
            ESP.restart();
        }
    }
    else 
    {
        lcd.print("Welcome to");
        lcd.setCursor(0, 1);
        lcd.print("Wash Connect");
        delay(2000);
    }
}

void updateDisplay(int col1, int row1, String message1, int col2, int row2, String message2,
                   int col3, int row3, String message3, int col4, int row4, String message4, int displayTime)
{
    lcd.clear();
    if(message1 != "-")
    {
        lcd.setCursor(col1, row1);
        lcd.print(message1);       
    }
    if(message2 != "-")
    {
        lcd.setCursor(col2, row2);
        lcd.print(message2);       
    }
    if(message3 != "-")
    {
        lcd.setCursor(col3, row3);
        lcd.print(message3);       
    }
    if(message4 != "-")
    {
        lcd.setCursor(col4, row4);
        lcd.print(message4);       
    }
    delay(displayTime*1000);
}

void setup() 
{
  // Initialize pins
  Serial.begin(9600);
  lcd.init(); // Initialize the LCD
  lcd.backlight();
  updateCoinDisplay(0, true);  
  pinMode(vm_detergent.pinLed, OUTPUT);
  pinMode(vm_detergent.pinButton, INPUT);
  pinMode(vm_detergent.pinButtonConfigure, INPUT);
  pinMode(vm_detergent.pinPump, OUTPUT);
  

  // put your setup code here, to run once:
}

void loop() 
{
  // Check button and blink LED if pressed
  vm_detergent.checkButtonAndBlink(vm_detergent.getPinButton(), vm_detergent.getPinLed());
  vm_softener.checkButtonAndBlink(vm_softener.getPinButton(), vm_softener.getPinLed());

  vm_detergent.checkButtonAndBlink(vm_detergent.getPinButtonConfigure(), vm_detergent.getPinPump());
  vm_softener.checkButtonAndBlink(vm_softener.getPinButtonConfigure(), vm_softener.getPinPump());
}