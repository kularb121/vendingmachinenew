#include <Arduino.h>
#include "VendingMachine.h"

// Initialize VendingMachine with pin numbers
VendingMachine vm_detergent(25, 33, 36, 12); //(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin)
VendingMachine vm_softener(32, 39, 34, 13);


void setup() 
{
  // Initialize pins
  Serial.begin(9600);
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
  // for(int i = 0; i <= 10; i++)
  // {
  //   digitalWrite(vm_detergent.pinPump, HIGH);
  //   digitalWrite(vm_detergent.pinLed, HIGH);
  //   digitalWrite(vm_softener.pinPump, HIGH);
  //   digitalWrite(vm_softener.pinLed, HIGH);     
  //   delay(200);
  //   digitalWrite(vm_detergent.pinPump, LOW);
  //   digitalWrite(vm_detergent.pinLed, LOW);
  //   digitalWrite(vm_softener.pinPump, LOW);
  //   digitalWrite(vm_softener.pinLed, LOW);
  //   delay(200);
  // } 
}