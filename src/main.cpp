#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "CoinAcceptor.h"
#include "VendingMachine.h"
#include "Mechanics.h"
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <SPI.h>
#include <PubSubClient.h> // Include the PubSubClient library
#include <ArduinoJson.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Math.h>
#include <HTTPUpdate.h>
// #include "AllArounds.h"
///////////////////////////////////////////////////////////////////////////////

//AllArounds AAS(25, 12, "vendingmachine_G101", "G101", "AAS", false, false);
//Ticker ticker, runTimeTick;
WiFiClient wifiClient, wifiClientFirmware;
WiFiManager wm;
PubSubClient mqttClient(wifiClient);

const char* ssid = "AewAew_2.4G";
const char* password = "meowmeow";

unsigned long previousMillisOngoing = 0;
const long intervalOngoing = 10000; // 10 seconds


// Initialize VendingMachine with pin numbers
Mechanics mechanics(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
CoinAcceptor coinAcceptor(35, 20, 221); // Initialize CoinAcceptor with pin 25
VendingMachine vm_detergent(25, 33, 36, 12); //(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin)
VendingMachine vm_softener(32, 39, 34, 13);

//void tick()             {  digitalWrite(AAS.pinReset, !digitalRead(AAS.pinReset)); }
//void ISRresetRunTime()  {  AAS.resetRunTime();   }

// void callback(char* topic, byte* payload, unsigned int length)  { AAS.callbackMqtt(topic, payload, length, mqttClient);}

// void attachTicker ()
// {
//   ticker.attach(0.2, tick);
//   runTimeTick.attach(60*60, ISRresetRunTime);
// }

void IRAM_ATTR coinInserted() {
    // Serial.println("Coin inserted");
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

// void WiFiTask(void *pvParameters) {
//     while (true) {
//         AAS.keepAlive(wm, mqttClient, wifiClientFirmware);
//         vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
//         esp_task_wdt_reset(); // Reset the watchdog timer
//     }
// }

// void WiFiTask(void *pvParameters) {
//     while (true) {
//         if (WiFi.status() != WL_CONNECTED) {
//             Serial.println("Wi-Fi disconnected, reconnecting...");
//             WiFi.begin(ssid, password);
//             unsigned long startAttemptTime = millis();

//             while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 12000) { // 12 seconds timeout
//                 vTaskDelay(500 / portTICK_PERIOD_MS); // Use vTaskDelay instead of delay
//                 Serial.print(".");
//                 esp_task_wdt_reset(); // Reset the watchdog timer
//             }

//             if (WiFi.status() == WL_CONNECTED) {
//                 Serial.println("Reconnected to Wi-Fi");
//             } else {
//                 Serial.println("Failed to reconnect to Wi-Fi within 12 seconds");
//             }
//         }

//         //AAS.keepAlive(wm, mqttClient, wifiClientFirmware);
//         vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
//         esp_task_wdt_reset(); // Reset the watchdog timer
//     }
// }

void setup() 
{
  // Initialize pins
  mechanics.init(); // Initialize the LCD

//   AAS.init(3, 10, "AAS IoT Manager", "AASIoT", "AASIoT12345", ticker, wm, wifiClientFirmware);
//   AAS.initMqtt(mqttClient, wm);
//   mqttClient.setCallback(callback);

//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to Wi-Fi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("Connected to Wi-Fi");

  mechanics.updateCoinDisplay(coinAcceptor.getCount(), true);
  attachInterrupt(digitalPinToInterrupt(coinAcceptor.getPinCoin()), coinInserted, RISING);
  attachInterrupt(digitalPinToInterrupt(vm_detergent.getPinButton()), detergentButtonISR, RISING);
  attachInterrupt(digitalPinToInterrupt(vm_softener.getPinButton()), softenerButtonISR, RISING);
  attachInterrupt(digitalPinToInterrupt(vm_detergent.getPinButtonConfigure()), detergentButtonConfigureISR, RISING);
  attachInterrupt(digitalPinToInterrupt(vm_softener.getPinButtonConfigure()), softenerButtonConfigureISR, RISING);

  // Initialize the watchdog timer
  esp_task_wdt_init(10, true); // Set timeout to 10 seconds
  esp_task_wdt_add(NULL); // Add the current task to the watchdog timer

  // Create FreeRTOS task for WiFi Manager and PubSubClient
//   xTaskCreatePinnedToCore(WiFiTask, "WiFiTask", 4096, NULL, 1, NULL, 1);
}

void printOngoing() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisOngoing >= intervalOngoing) {
        previousMillisOngoing = currentMillis;
        Serial.println("Ongoing");
    }
}

void loop() 
{
  // Reset the watchdog timer
  esp_task_wdt_reset();
  printOngoing();
  // Check button and blink LED if pressed.
  mechanics.updateCoinDisplay(coinAcceptor.getCount(), false);
  vm_detergent.handleAllButtonPresses(coinAcceptor.count);
  vm_softener.handleAllButtonPresses(coinAcceptor.count);
  coinAcceptor.checkAndResetCount(); // Check and reset the coin count if more than five minutes have passed
}