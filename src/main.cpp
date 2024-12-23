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

// Initialize VendingMachine with pin numbers
Mechanics mechanics(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
CoinAcceptor coinAcceptor(35, 20); // Initialize CoinAcceptor with pin 25
int regPump1Time = 311;
int regPump2Time = 312;
VendingMachine vm_detergent(25, 33, 36, 12, regPump1Time); //(int ledPin, int buttonPin, int buttonConfigurePin, int pumpPin)
VendingMachine vm_softener(32, 39, 34, 13, regPump2Time);

void tick()             {  digitalWrite(AAS.pinReset, !digitalRead(AAS.pinReset)); }
void ISRresetRunTime()  {  AAS.resetRunTime();   }

void callback(char* topic, byte* payload, unsigned int length)  { AAS.callbackMqtt(topic, payload, length, mqttClient);}

void attachTicker ()
{
  ticker.attach(0.2, tick);
  runTimeTick.attach(60*60, ISRresetRunTime);
}

// Debouncing variables
unsigned long lastPulseTime = 0;
volatile int pulseCount = 0;
const unsigned long debounceDelay = 40; // 40 milliseconds
void IRAM_ATTR coinInserted() {
  unsigned long currentTime = millis();

  if (currentTime - lastPulseTime > debounceDelay) {
    coinAcceptor.coinInsertedFlag = true;
    pulseCount++;
    coinAcceptor.lastCoinTime = currentTime;
    lastPulseTime = currentTime;
  }
}

void WiFiTask(void *pvParameters) {
    int retryCount = 0;
    const int maxRetries = 10;

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
  AAS.regPump1Time = regPump1Time;
  AAS.regPump2Time = regPump2Time;
  mqttClient.setCallback(callback);

  // Initialize the coin count from EEPROM.
  // It reads the stored coin count value from EEPROM and initializes the coin count variable.
  // It ensures that the coin count is persistent across device restarts by loading the previously saved count.
  coinAcceptor.initCount();
  mechanics.updateCoinDisplay(coinAcceptor.getCount(), true, false);
  attachInterrupt(digitalPinToInterrupt(coinAcceptor.getPinCoin()), coinInserted, RISING);

  // Initialize the watchdog timer
  esp_task_wdt_init(30, true); // Set timeout to 10 seconds
  esp_task_wdt_add(NULL); // Add the current task to the watchdog timer

  // Create FreeRTOS task for WiFi Manager and PubSubClient
   xTaskCreatePinnedToCore(WiFiTask, "WiFiTask", 4096, NULL, 1, NULL, 1);

}

void loop() 
{
  // Reset the watchdog timer
  esp_task_wdt_reset();

  // old version
  //mechanics.updateCoinDisplay(coinAcceptor.getCount(), false);
  coinAcceptor.handleCoinInsertion(mechanics, pulseCount);
  int detergentCoins = vm_detergent.handleAllButtonPresses(coinAcceptor.count);
  int softenerCoins = vm_softener.handleAllButtonPresses(coinAcceptor.count);
  int resetCoins = coinAcceptor.checkAndResetCount(); // Check and reset the coin count if more than five minutes have passed
  AAS.keepAliveReport(wm, mqttClient, wifiClient, detergentCoins, softenerCoins, resetCoins);
}