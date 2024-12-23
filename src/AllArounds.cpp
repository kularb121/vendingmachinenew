#include "AllArounds.h"
#include <esp_task_wdt.h>

AllArounds::AllArounds() {}

AllArounds::AllArounds(byte pinReset, byte pinSWReset, String fileName, String fileVersion, String customer, bool reqTestRun, bool printStatus)    
{
  this->pinReset = pinReset; //equals to pinLed of Detergent 
  this->pinSWReset = pinSWReset;   //equals to pinButton of Detergent.
  this->fileName = fileName;
  this->fileVersion = fileVersion;
  this->reqTestRun = reqTestRun;
  this->printStatus = printStatus;
  shortResetTime = 1;
  longResetTime = 15;
  this->customer = customer;
  this->updateFileLoc  = "http://allarounds.online/" + customer + "/" + fileName + ".bin";
  changePass = "yXd196@t";  
}

void AllArounds::init(byte delayAtStart, float updateFirmwareCheck, String apTitle, String apName, String apPass, Ticker &ticker, WiFiManager &wm, WiFiClient &wifiClient)
{
  delay(delayAtStart*1000);
  Serial.begin(9600);
  EEPROM.begin(512);
   
  this->updateFirmwareCheck = updateFirmwareCheck;
  this->apTitle = apTitle;
  this->apName = apName + String(EEPROM.read(regMachGreaterIden)*256 + EEPROM.read(regMachLesserIden));
  this->apPass = apPass;
  Serial.println("Before set-up");
  setupWM(ticker, wm, wifiClient);
  firstTimeSetUp();
  devName = getDevNumber();  
  clientId = devName + "-" + String(random(0xffff), HEX);
  if(EEPROM.read(regEnableUpdate) <= 1)
    enableUpdate = EEPROM.read(regEnableUpdate);
  else
    enableUpdate = 0;
}

void AllArounds::initMqtt(PubSubClient &mqttClient, WiFiManager &wm)
{
  this->mqtt_server = "34.71.10.73";
  this->mqtt_userName = "mqtt.allarounds"; 
  this->mqtt_password = "2IL$rH74Odz0";
  this->sendTopic = customer + "/mcu/" + devName;
  this->receiveTopic = customer + "/app/" + devName;
  this->toServer = "AASMonitor";  
  this->updateMqttPeriod = 2;
  this->pingMqttPeriod = 5;
  mqttClient.setServer(mqtt_server.c_str(), 1883);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(240);
  reconnectMqtt(mqttClient, wm); 
  publishMQTT(mqttClient, "fileName", fileName, fileVersion);
  String usageAuth = "Yes";
  if(machAuth == 1)
    usageAuth = "No";
  publishMQTT(mqttClient, "machAuth", usageAuth, "0");
  reportLastErrors();      
}

bool AllArounds::reconnectMqtt(PubSubClient &mqttClient, WiFiManager &wm) 
{
  int maxAttempts = 10;  
  while (!mqttClient.connected()) 
  {
    for(int i = 1; i <= maxAttempts; i++)
    {
      reconnectWifi(wm);
      Serial.print("Attempting MQTT connection...");
      delay(10);
      esp_task_wdt_reset(); // Reset the watchdog timer
      if (mqttClient.connect(clientId.c_str(), mqtt_userName.c_str(), mqtt_password.c_str())) //
      {
        Serial.println("connected");
        mqttClient.subscribe(receiveTopic.c_str());
        return true;      
      } 
      else 
      {
        Serial.println("failed, rc=" + String(mqttClient.state()) + " try again in 2 seconds");
        Serial.println("WiFi status is " + WiFi.status());
        delay(2000);
      }
      if(i == maxAttempts)
      {

        resetupMqttClient(mqttClient);
        return false;       
      }
    }
  }
  if (mqttClient.connected())
  {
    mqttClient.loop();
    return true;     
  }
  else 
    return false;
}

void AllArounds::resetupMqttClient(PubSubClient &mqttClient)
{
  mqttClient.setServer(mqtt_server.c_str(), 1883);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(240); 
}
void AllArounds::setup_wifi(WiFiManager &wm) 
{
  WiFi.mode(WIFI_STA);  
  reconnectWifi(wm);  
}

bool AllArounds::reconnectWifi(WiFiManager &wm) 
{
  int i = 0;
  int maxTries = 5;
  while ((WiFi.status() != WL_CONNECTED) && (i < maxTries))
  {
    if (wm.autoConnect(apName.c_str(), apPass.c_str()))
    {
      printToDebug("Wifi is connected");
      return true;         
    }
    Serial.print(".");
    printToDebug("WiFi Disconnected, retry in 2 seconds");    
    delay(2000);
    esp_task_wdt_reset(); // Reset the watchdog timer
    i++;
  }   
  if( i >= maxTries )
    return false;
  else
    return true;
  return true;
}

void AllArounds::printToDebug(String printMsg)
{
  if(printStatus)
  {
    Serial.println(printMsg);
    delay(20);
  }  
}

// Miscellaneouse operations
void AllArounds::resetESP(bool checkCond, String reportMessage)
{
  if(checkCond)
  {
    printToDebug("Reset condition | " + reportMessage + " is met, system is restarting...");
    Serial.println("Reset is starting");
    ESP.restart();   
  }
}

void AllArounds::checkFirmwareUpdate(WiFiClient &wifiClient, WiFiManager &wm)
{
  reconnectWifi(wm);
  if (timeMinuteCheck(timerFirmware, updateFirmwareCheck, 0)) 
  {
    t_httpUpdate_return ret = httpUpdate.update(wifiClient, updateFileLoc);
    switch (ret) 
    {
      case HTTP_UPDATE_OK:
        reconnectWifi(wm);
        EEPROMWriteESP(regEnableUpdate, 0);
//        Serial.println("HTTP_UPDATE_OK");
        ESP.restart();
        break;
    }
  }    
}

void AllArounds::EEPROMWriteESP(int currReg, int writeValue)
{
  EEPROM.write(currReg, writeValue);
  EEPROM.commit();
  delay(20);
  printToDebug("The EEPROM of " + String(currReg) + " is now " + EEPROM.read(currReg));
  delay(10);  
}

void AllArounds::clearSerialBuffer()
{      
  Serial.flush();
  while (Serial.available() > 0)
  {
    delay(20);
    char inChar = (char)Serial.read(); 
  }            
  delay(20);
}

void AllArounds::ISRwdMqtt() {
  wdMqtt++;             //  Serial.println("MQTT is being checked, now is " + String(wdMqtt));
  if (wdMqtt == wdReset) {
    Serial.println("MQTT disconnected, to restart.");
    EEPROM.write(regWdMqtt, 1);
    EEPROM.commit();
    ESP.restart();
  }
}
void AllArounds::ISRwdWifi() {
  wdWifi++;             //  Serial.println("WiFi is being checked, now is " + String(wdWifi));
  if (wdWifi == wdReset) {
    Serial.println("Wifi disconnected, to restart.");
    EEPROM.write(regWdWifi, 1);
    EEPROM.commit();
    ESP.restart();
  }  
}

String AllArounds::lastErrorMessage(int regIndex)
{
  switch (regIndex) {
    case 1:
      return " Wifi error ";
    case 2:
      return " Mqtt error ";
    default:
      return "-";
  }
  
}

void AllArounds::watchDog(PubSubClient &mqttClient) 
{
  if(mqttClient.connected()) {
    wdMqtt = 0;
  }
  if((WiFi.status() == WL_CONNECTED)) {
    wdWifi = 0;
  }  
}

void AllArounds::reportLastErrors()
{
  int i = regWdStart;
  while (i <= regWdFinal)
  {
    if(EEPROM.read(i) == 1)
    {

    }
    i+= 1;
  }
}

bool AllArounds::publishMQTT(PubSubClient &mqttClient, String message, String machStatus, String amount)
{
  StaticJsonDocument<384> repDoc;
  repDoc["to"] = toServer;
  repDoc["from"] = devName; //devName + toMach.substring(15)
  repDoc["message"] = message;
  repDoc["status"] = machStatus;
  repDoc["amount"] = amount;  
  String output;
  serializeJson(repDoc, output);
  delay(10);
  char msg2[MSG_BUFFER_SIZE];
  snprintf (msg2, MSG_BUFFER_SIZE, output.c_str());
  delay(20);    
  if(mqttClient.publish(sendTopic.c_str(), msg2))    //output.c_str()
  {
    delay(50);
    printToDebug("Publish is successful!");
    return true;
  }
  else 
    return false;    
}

bool AllArounds::isChanged(int changeMode, float &timeLatest, float updatePeriod, double &currValue, double newValue)
{
  int threshold = 1500;
  double changeThreshold = 0.05;
  // int thresholdMinAds = 700;
  // int thresholdHighAds = 8000;
  //Set update period based on ads reading.
  if(newValue > thresholdHighAds)
    updatePeriod = updatePeriod / updateRatio;

  // if(currValue < 0 || newValue < 0)
  //   return false;
  switch (changeMode)
  {
    case 0: //Update by referring to time only.
      if((double(millis())/60000.0 - timeLatest >= 0)  &&  (double(millis())/60000.0 >= firstDelay))
      {
        timeLatest = double(millis())/60000.0 + updatePeriod;
        return true;
      }
      else 
        return false;
      break;
   case 1: //Update by referring to change only
      if(currValue != newValue)
      {              
        currValue = newValue;               
        return true;
      }
      else 
        return false;
      break;
  case 2: //Update by referring to time or change.
      double timeDiff = double(millis())/60000.0 - timeLatest;
      if(timeDiff >= updatePeriod)
      {
        timeLatest = double(millis())/60000.0 + updatePeriod;
        currValue = newValue;                     
        return true; 
      }
      else if( currValue < threshold && newValue < threshold)
      {
        return false;
      }
      else if( currValue < threshold && newValue > threshold)
      {
        timeLatest = double(millis())/60000.0 + updatePeriod;
        currValue = newValue;                     
        return true;         
      }
      else if( currValue > threshold && newValue > threshold)
      {
        if(abs(currValue - newValue) > changeThreshold * currValue)
        {
          timeLatest = double(millis())/60000.0 + updatePeriod;
          currValue = newValue;      
          return true; 
        }
        else
          return false;
      }
      else if( (currValue > threshold) && (newValue < threshold) && (newValue > 0))
      {
        timeLatest = double(millis())/60000.0 + updatePeriod;
        currValue = newValue;                     
        return true; 
      }  
      else 
        return false;
      break;      
  }
  return false;
}

bool AllArounds::isTimeUpdate(double &timeLatest, float updatePeriod)
{
  if(double(millis())/60000.0 - timeLatest >= 0)
  {
    timeLatest = double(millis())/60000.0 + updatePeriod;
    return true;
  }
  else 
    return false;
  return false;
}

bool AllArounds::software_Reboot() 
{
  if((float(millis())/3600000.0 - resetTime) >0) 
  {
    delay(3000);
    Serial.println("Restarting.....................");
    digitalWrite(pinRelay, HIGH);
    delay(15000);
    digitalWrite(pinRelay, LOW);
    ESP.restart();
    return true;
  }
  else 
    return false;
} 

void AllArounds::setupWM(Ticker &ticker, WiFiManager &wm, WiFiClient &wifiClient)
{
  wm.setConnectTimeout(10);
  wm.setConnectRetries(5);
  wm.setWiFiAutoReconnect(true);
  wm.setConfigPortalTimeout(60);                                           
  wm.setTitle(apTitle.c_str());
  wm.setConfigPortalBlocking(true);
  wm.disconnect();

  // Retry mechanism
  const int maxRetries = 5; // Retry for 30 seconds
  int retryCount = 0;
  bool connected = false;

  while (retryCount < maxRetries && !connected) {
    if (wm.autoConnect(apName.c_str(), apPass.c_str())) {
      Serial.println("Connected to Wi-Fi using saved credentials");
      connected = true;
    } else {
      Serial.println("Failed to connect, retrying...");
      retryCount++;
      delay(1000); // Wait for 1 second before retrying
    }
  }

  if (!connected) {
    Serial.println("Failed to connect using saved credentials, starting configuration portal...");
    wm.startConfigPortal(apName.c_str(), apPass.c_str());
  }

  wm.startWebPortal();   

  checkResetWiFi(wm);    
  setup_wifi(wm);
  wm.setConfigPortalTimeout(10); // To prevent others from accessing during normal operation, shorten idle time
  digitalWrite(pinReset, LOW);
  ticker.detach();
  checkFirmwareUpdate(wifiClient, wm);   
}



void AllArounds::checkResetWiFi(WiFiManager &wm)
{
  if(digitalRead(pinSWReset) == HIGH) 
  {
    printToDebug("Wifi Reset? Pls. waiting 3S..");
    delay(3000);
    if (digitalRead(pinSWReset) == HIGH) 
    {
      printToDebug("WiFi Reset is starting....");
      wm.resetSettings();                                         //      ESP.restart(); 
      wm.setTitle(apTitle.c_str());
      if( !wm.autoConnect(apName.c_str(), apPass.c_str()))
      {
        printToDebug("Failed to connect and hit timeout");
        ESP.restart();
        delay(500);  
      }
      Serial.println("Connected to WiFi.");
    }
  }        
}

void AllArounds::keepAlive(WiFiManager &wm, PubSubClient &mqttClient, WiFiClient &wifiClient)
{
  reconnectWifi(wm);
  reconnectMqtt(mqttClient, wm);
  mqttClient.loop();
  watchDog(mqttClient);
  software_Reboot();
  checkFirmwareUpdate(wifiClient, wm);
}

void AllArounds::keepAliveReport(WiFiManager &wm, PubSubClient &mqttClient, WiFiClient &wifiClient, int detergentCoins, int softenerCoins, int resetCoins)
{
  keepAlive(wm, mqttClient, wifiClient);
  if(detergentCoins > 0)
  {
    publishMQTT(mqttClient, "coins", "detergent", String(detergentCoins));
  }

  if(softenerCoins > 0)
  {
    publishMQTT(mqttClient, "coins", "softener", String(softenerCoins));
  }
  
  if(resetCoins > 0)
  {
    publishMQTT(mqttClient, "coins", "reset", String(resetCoins));
  }
}

void AllArounds::reportRSSI(WiFiManager &wm, PubSubClient &mqttClient)
{
  reconnectWifi(wm);
  reconnectMqtt(mqttClient, wm);
  publishMQTT(mqttClient, "RSSI", String(WiFi.RSSI()), "0");
}

void AllArounds::callbackMqtt(char* topic, byte* payload, unsigned int length, PubSubClient &mqttClient)
{
  Serial.print("Message arrived [" + String(topic) + "] ");
  String json;
  for (int i = 0; i < length; i++) 
    json += (char)payload[i];
  Serial.println();  
  StaticJsonDocument<384> doc;
  DeserializationError error = deserializeJson(doc, json.c_str());
  if (error) 
  {
    Serial.println(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  String toMach = doc["to"];        
  String fromDev = doc["from"];      
  String message = doc["message"];   
  String machStatus = doc["status"];
  String machAmount = doc["amount"];

  if (message == "getEEPROMvalue") {
    handleGetEEPROMValue(machStatus, machAmount, mqttClient);
  } else if (message == "getDeviceNumber") {
    handleGetDeviceNumber(mqttClient);
  } else if (message == "setEnableUpdate") { 
    handleSetEnableUpdate(machStatus, mqttClient);
  } else if (message.equals("fileName")) {
    handleFileName(mqttClient);
  } else if (message == "changeDeviceNumber") {
    handleChangeDeviceNumber(machStatus, machAmount, mqttClient);
  } else if (message == "scanWiFi") {
    handleScanWiFi(mqttClient);
  } else if (message == "setEEPROMvalue") {
    handleSetEEPROMValue(machStatus, machAmount, mqttClient);
  } else if (message == "resetDevice") {
    handleResetDevice(machStatus, machAmount);
  } else if (message == "getEnableUpdate") {
    handleGetEnableUpdate(mqttClient);
  } else if (message == "setPumpTime") { 
    handleSetPumpTime(machStatus, machAmount, mqttClient);
  } else if (message == "getPumpTime") { 
    handleGetPumpTime(machStatus, machAmount, mqttClient);
  } else {
    Serial.println("No correct message is found.");
  } 
}

String AllArounds::changeDeviceNumber(int newDevNumber)
{
    if(newDevNumber > 0 && newDevNumber < 1000)
    {
      setDevNumber(newDevNumber);
      return getDevNumber();              
    }
    else
      return "Out of Range"; 
}

String AllArounds::getDevNumber()
{ 
  int iotNumber = (EEPROM.read(regMachGreaterIden))*256 + EEPROM.read(regMachLesserIden);
  String tmpDev;
  String devPrefix = "AASVM-";
  if(iotNumber < 10)
    tmpDev = devPrefix + "00" + String(iotNumber);
  else if (iotNumber < 100 && iotNumber >= 10)
    tmpDev = devPrefix + "0" + String(iotNumber);
  else if (iotNumber >= 100)
    tmpDev = devPrefix  + String(iotNumber);
//  printToDebug("Device number is " + tmpDev);
  return tmpDev;    
}

void AllArounds::setDevNumber(int devNumber)
{
  int lesserNumber = devNumber % 256;
  int greaterNumber = devNumber / 256;
  EEPROMWriteESP(regMachLesserIden, lesserNumber);
  EEPROMWriteESP(regMachGreaterIden, greaterNumber);  
}

void AllArounds::firstTimeSetUp()
{
  if(EEPROM.read(regFirstSetup) != 199)
  {
    setEEPROM();
    setDevNumber(999);
  }
}

void AllArounds::setEEPROM()
{
  for(int i = 0; i <= 500; i++)
    EEPROMWriteESP(i, 0);
  EEPROMWriteESP(regFirstSetup, 199);  
}

void AllArounds::resetRelay(int timeDelayMinutes) {
  Serial.println("Relay is pressed, to restart");
  digitalWrite(pinRelay, HIGH);
  delay(timeDelayMinutes*60*1000);
  digitalWrite(pinRelay, LOW);
  ESP.restart();
}

void AllArounds::resetRunTime()
{
  currRunTime++;             //  Serial.println("MQTT is being checked, now is " + String(wdMqtt));
  Serial.println("currRunTime = " + String(currRunTime));
  if (currRunTime >= totalRunTime)
  {
    resetRelay(15);
    ESP.restart();
  }
}

bool AllArounds::timeMinuteCheck(float &timer, float nextUpdate, float firstDelay)
{
  if((float(millis())/60000.0 - timer >= 0)  &&  (float(millis())/60000.0 >= firstDelay))
  {
    timer = float(millis())/60000.0 + nextUpdate;
    return true;
  }
  else 
    return false;
}

void AllArounds::resetMCUMemory(int resetMode)
{
  switch(resetMode)
  {
    case 1:
      for(int i = 100; i <= 122; i++)
        EEPROMWriteESP(i, 0);
      EEPROMWriteESP(202, 0);   // 202  MControl.countOverhaul
      EEPROMWriteESP(201, 0);      // MControl.countScrew

      break;
    case 2:
      for(int i = 130; i <= 142; i++)
        EEPROMWriteESP(i, 0);
      break;      
  }
}

void AllArounds::handleGetEEPROMValue(String machStatus, String machAmount, PubSubClient &mqttClient) {
  int currReg = machStatus.toInt();
  String result1, result2; 
  if (currReg > 0 && currReg <= 500) {
    result1 = machStatus;
    result2 = String(EEPROM.read(currReg));
  } else {
    result1 = machStatus;
    result2 = "Out of range";
  }
  publishMQTT(mqttClient, "getEEPROMvalue", result1, result2); 
}

void AllArounds::handleGetDeviceNumber(PubSubClient &mqttClient) {
  String result1 = getDevNumber();
  publishMQTT(mqttClient, "getDeviceNumber", result1, "0"); 
}

void AllArounds::handleSetEnableUpdate(String machStatus, PubSubClient &mqttClient) {
  int getUpdate = machStatus.toInt();
  String result1, result2;
  if (getUpdate <= 1) {
    EEPROMWriteESP(regEnableUpdate, getUpdate);
    enableUpdate = EEPROM.read(regEnableUpdate);
    result1 = machStatus;
  } else {
    result1 = machStatus; 
  }
  result2 = String(EEPROM.read(regEnableUpdate)); 
  publishMQTT(mqttClient, "setEnableUpdate", result1, result2);
}

void AllArounds::handleFileName(PubSubClient &mqttClient) {
  publishMQTT(mqttClient, "fileName", fileName, fileVersion);
}

void AllArounds::handleChangeDeviceNumber(String machStatus, String machAmount, PubSubClient &mqttClient) {
  if (!machAmount.equals(changePass)) {
    publishMQTT(mqttClient, "changeDeviceNumber", machStatus, "Incorrect Password");
    return; 
  }

  String changeDevResult = changeDeviceNumber(machStatus.toInt());
  publishMQTT(mqttClient, "changeDeviceNumber", machStatus, changeDevResult);

  if (changeDevResult != "Out of Range") {
    delay(5000); 
    ESP.restart();
  }
}

void AllArounds::handleScanWiFi(PubSubClient &mqttClient) {
  int numberOfNetworks = WiFi.scanNetworks();
  int channel[15] = {0}; // Initialize all elements to 0

  for (int i = 0; i < numberOfNetworks; i++) {
    int channelThreshold = -70;
    if (WiFi.RSSI(i) > channelThreshold) {
      channel[WiFi.channel(i)] += 1;
    }
  }

  for (int i = 1; i <= 11; i++) {
    publishMQTT(mqttClient, "scanWiFi", String(i), String(channel[i]));
    delay(100); 
  }
  publishMQTT(mqttClient, "scanWiFi", "currChannel", String(WiFi.channel()));
}

void AllArounds::handleSetEEPROMValue(String machStatus, String machAmount, PubSubClient &mqttClient) {
  int currReg = machStatus.toInt();
  String result1, result2;
  if (currReg > 0 && currReg <= 500) {
    result1 = machStatus;
    int setVal = machAmount.toInt();
    if (setVal >= 0 && setVal <= 255) {
      EEPROMWriteESP(currReg, setVal);
      result1 = machStatus;      
      result2 = String(EEPROM.read(currReg));
    } else {
      result1 = machStatus;
      result2 = "999"; // Indicate invalid value
    }
  } else {
    result1 = machStatus;
    result2 = "Out of range";
  }
  publishMQTT(mqttClient, "setEEPROMvalue", result1, result2);
}

void AllArounds::handleResetDevice(String machStatus, String machAmount) {
  if (machAmount.equals(changePass)) {    
    resetRelay(machStatus.toInt());
  }  
}

void AllArounds::handleGetEnableUpdate(PubSubClient &mqttClient) {
  String result1 = String(EEPROM.read(regEnableUpdate));
  publishMQTT(mqttClient, "getEnableUpdate", result1, "0");
}

void AllArounds::handleSetPumpTime(String machStatus, String machAmount, PubSubClient &mqttClient) {
  int regPumpTime = 0;
  String result1, result2;
  if(machStatus == "pump1")
  {
    regPumpTime = 311;
  }
  else if (machStatus == "pump2")
  {
    regPumpTime = 312;
  }

  if (regPumpTime > 0) 
  {
    result1 = machStatus;
    int setVal = machAmount.toInt();
    if (setVal >= 0 && setVal <= 255) {
      EEPROMWriteESP(regPumpTime, setVal);
      result1 = machStatus;      
      result2 = String(EEPROM.read(regPumpTime));
    } else {
      result1 = machStatus;
      result2 = "999"; // Indicate invalid value
    }
  } else {
    result1 = machStatus;
    result2 = "Out of range";
  }

  publishMQTT(mqttClient, "setPumpTime", result1, result2);
}

void AllArounds::handleGetPumpTime(String machStatus, String machAmount, PubSubClient &mqttClient) 
{
  if(machStatus == "pump1")
  {
    String result1 = String(EEPROM.read(regPump1Time));
    publishMQTT(mqttClient, "getPumpTime", machStatus, result1);
  }
  else if (machStatus == "pump2")
  {
    String result1 = String(EEPROM.read(regPump2Time));
    publishMQTT(mqttClient, "getPumpTime", machStatus, result1);
  }
}