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
#include "AllArounds.h"
///////////////////////////////////////////////////////////////////////////////

AllArounds AAS(25, 12, "vendingmachine_G101", "G101", "AAS", false, false);
Ticker ticker, runTimeTick;
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

void tick()             {  digitalWrite(AAS.pinReset, !digitalRead(AAS.pinReset)); }
void ISRresetRunTime()  {  AAS.resetRunTime();   }

void callback(char* topic, byte* payload, unsigned int length)  { AAS.callbackMqtt(topic, payload, length, mqttClient);}

void attachTicker ()
{
  ticker.attach(0.2, tick);
  runTimeTick.attach(60*60, ISRresetRunTime);
}

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


void WiFiTask(void *pvParameters) {
    int retryCount = 0;
    const int maxRetries = 5;

    while (true) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi disconnected, reconnecting...");
            if (!wm.autoConnect((AAS.apName).c_str(), (AAS.apPass).c_str())) {
                Serial.println("Failed to connect and hit timeout");
                retryCount++;
                if (retryCount >= maxRetries) {
                    Serial.println("Max retries reached, continuing without Wi-Fi");
                    retryCount = 0; // Reset retry count
                }
            } else {
                Serial.println("Connected to Wi-Fi");
                retryCount = 0; // Reset retry count on successful connection
            }
        }

        AAS.keepAlive(wm, mqttClient, wifiClientFirmware);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
        esp_task_wdt_reset(); // Reset the watchdog timer
    }
}

void setup() 
{
  // Initialize pins
  mechanics.init(); // Initialize the LCD

  AAS.init(3, 10, "AAS IoT Manager", "AASIoT", "AASIoT12345", ticker, wm, wifiClientFirmware);
  AAS.initMqtt(mqttClient, wm);
  mqttClient.setCallback(callback);

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
   xTaskCreatePinnedToCore(WiFiTask, "WiFiTask", 4096, NULL, 1, NULL, 1);
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
  int detergentCoins = vm_detergent.handleAllButtonPresses(coinAcceptor.count);
  int softenerCoins = vm_softener.handleAllButtonPresses(coinAcceptor.count);
//   vm_detergent.handleAllButtonPresses(coinAcceptor.count);
//   vm_softener.handleAllButtonPresses(coinAcceptor.count);
  int resetCoins = coinAcceptor.checkAndResetCount(); // Check and reset the coin count if more than five minutes have passed
  AAS.keepAlive(wm, mqttClient, wifiClient);
  if(detergentCoins > 0)
  {
    AAS.publishMQTT(mqttClient, "coins", "detergent", String(detergentCoins));
  }

  if(softenerCoins > 0)
  {
    AAS.publishMQTT(mqttClient, "coins", "softener", String(softenerCoins));
  }

  if(resetCoins > 0)
  {
    AAS.publishMQTT(mqttClient, "coins", "reset", String(resetCoins));
  }
}