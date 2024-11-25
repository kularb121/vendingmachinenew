#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "CoinAcceptor.h"
#include "VendingMachine.h"
#include "Mechanics.h"

// Initialize VendingMachine with pin numbers
Mechanics mechanics(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
CoinAcceptor coinAcceptor(35, 20, 221); // Initialize CoinAcceptor with pin 25
VendingMachine vm_detergent(25, 33, 36, 12); //(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin)
VendingMachine vm_softener(32, 39, 34, 13);

void IRAM_ATTR coinInserted() {
    coinAcceptor.incrementCount();
}

void IRAM_ATTR detergentButtonISR() {
    vm_detergent.setButtonPressed(true);
}

void IRAM_ATTR softenerButtonISR() {
    vm_softener.setButtonPressed(true);
}

void IRAM_ATTR detergentButtonConfigureISR() {
    vm_detergent.setButtonConfigurePressed(true);
}

void IRAM_ATTR softenerButtonConfigureISR() {
    vm_softener.setButtonConfigurePressed(true);
}

void setup() 
{
  // Initialize pins
  mechanics.init(); // Initialize the LCD
  mechanics.updateCoinDisplay(coinAcceptor.getCount(), true);
  attachInterrupt(digitalPinToInterrupt(coinAcceptor.getPinCoin()), coinInserted, FALLING);
  attachInterrupt(digitalPinToInterrupt(vm_detergent.getPinButton()), detergentButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(vm_softener.getPinButton()), softenerButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(vm_detergent.getPinButtonConfigure()), detergentButtonConfigureISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(vm_softener.getPinButtonConfigure()), softenerButtonConfigureISR, FALLING);
}

void loop() 
{
  // Check button and blink LED if pressed.
  mechanics.updateCoinDisplay(coinAcceptor.getCount(), false);
  vm_detergent.handleAllButtonPresses(coinAcceptor.count);
  vm_softener.handleAllButtonPresses(coinAcceptor.count);
}