#ifndef ALLAROUNDS_H 
#define ALLAROUNDS_H

#include <WiFi.h>
#include <SPI.h>
#include <Arduino.h>
#include <HTTPUpdate.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>
#include <Math.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

class AllArounds
{
  private:
    // TridentTD_LineNotify LINE;
  public:
  //Initialize hardware-related components
    AllArounds();
    AllArounds(byte pinReset, byte pinSWReset, String fileName, String fileVersion, String customer, bool reqTestRun, bool printStatus);

  //Variables
  
    //Tracking health of the system.
      volatile int wdMqtt = 0;
      volatile int wdWifi = 0;
      int isrPeriod = 2*60;
      int hbPeriod = 10*60;
      int wdReset = 150*60 / isrPeriod;            //Default reset of watchdog = 10 mins.
//      int currPeriodReg = 500; 
      int shortResetTime, longResetTime;      
    //Use for input/output of MCU
      byte pinSWReset, pinReset;
      byte pinRelay = 15;  
    //Device info 
      String devName, toServer, clientId, newDevDame, newFileName;
      String fileName, fileVersion, updateFileLoc, customer;
      String apTitle, apName, apPass;
    //RunTime management
      int currRunTime = 0;
      int totalRunTime = 24;
    //Communicating with internet services 
      //LINE
        String LINE_TOKEN, LINE_TOKEN_MONITOR;

      //Initial State
        float timerIS, updateISperiod;
        byte firstDelay = 0;
        String bucketName; // Bucket name (name your data will be associated with in Initial State):  
        String bucketKey;
        String accessKey;                            
      //Others
        float updateStatusPeriod, updateFirmwareCheck;
    //MQTT related
        String mqtt_server, mqtt_userName, mqtt_password; 
        String branchCode;
        String receiveTopic;
        String sendTopic;
        String replyToAll;
        int MSG_BUFFER_SIZE = 384;
        float timerMqtt;
        float updateMqttPeriod, pingMqttPeriod;                
    //Miscellaneous operations
      int relayTestMax = 10;
      bool printStatus;
      bool reqTestRun = false;
      float resetTime = 24;
      float timerFirmware;
      byte countSerialFail = 0;
      byte maxSerialFail = 3;
      byte currSerial = 0;    
      String changePass;
    //EEPROM Registers to record long-term values.
      byte regWdStart = 11;
      byte regWdFinal = 13;
      byte regWdWifi = 11;
      byte regWdMqtt = 12;
      byte regWdMsgMqtt = 13;
      byte regWdRunTime = 14;
      byte regMachGreaterIden = 29;
      byte regMachLesserIden = 30;
      byte regEnableUpdate = 51;

    //EEPROM Registers for RX2 and TX2
      byte regRX2 = 61;
      byte regTX2 = 62;
      byte RX2pinNum;
      byte TX2pinNum;

      byte maxAdsFail = 30;
      byte regAds0 = 91;
          
      int regSerial = 499;
      int regFirstSetup = 501;
      byte enableUpdate;

      bool functionTestState = false;
      float timerFunction = 0;
      float updateFunction = 0.5;

    //// Run Time and Load Time;
      float currentNow = 0;
      float runTime = 0;
      float loadTime = 0;

    /////////////////////////////////////////////////////////////////
    //Hour Meter
    //Hour meter's variables EEPROM address
      int addrMin1 = 102; int addrHour1B = 101; int addrHour1A = 100; 
      int updateCycle1 = 2;
      float currTime1 = 0;
      int addrMin2 = 112; int addrHour2B = 111; int addrHour2A = 110; 
      int updateCycle2 = 3;
      float currTime2 = 0;
      int addrMin3 = 122; int addrHour3B = 121; int addrHour3A = 120;
      int updateCycle3 = 5;
      float currTime3 = 0;    
      int addrMin4 = 132;int addrHour4B = 131; int addrHour4A = 130;
      int addrMin5 = 142;int addrHour5B = 141; int addrHour5A = 140;
      
      int addPeriod = 1; // 1 minute
      int periodCount = 6;
      int periodRounds = 60 / periodCount;

    //Monitor opening the boxes.
    //Monitor opening of the boxes.
      byte pinSWiot = 32;     
      byte pinSWcontrol = 33;
      byte pinMachinePower = 36; 
      byte stateSWiot = 0;
      byte stateSWcontrol = 0;
      bool reportSWiot = false;
      bool reportSWcontrol = false;
      bool reportFirstState = true;
    //Manage pins for modules.
    //Relay to manage activities
      byte pinRouter = 25;
    //  byte pinADC = 26;
      byte pinGPS = 26;
      bool isRouter = false;
      bool isADC = false;
      bool isGPS = false;

    //States
      bool plugState = true;
    //Pins

      //byte pinWhite = 25;
      //byte pinGreen = 26;
      byte pinYellow = 27;
      byte pinBlue = 14;
      byte pinMachine = 4;
      byte addrMachAuth = 201;
      byte machAuth = false;
    //Monitoring currents to motors, reporting times and GPS.
      float tsCurrent, tsTime, tsGPS;
      float updateCurrent = 2.0;
      float updateTsTime = 2;
      float updateGPS = 2;
      int thresholdMinAds = 700;
      int thresholdHighAds = 8000;
      int updateRatio = 10;
      float current1 = 0, current2 = 0, current3 = 0;

      float tsCurrentV0, tsCurrentV1, tsCurrentV2, tsCurrentV3;
      float tsCurrentH0, tsCurrentH1, tsCurrentH2, tsCurrentH3;      
      double currentH0_last = 0, currentH1_last = 0, currentH2_last = 0, currentH3_last = 0;
      double currentV0_last = 0, currentV1_last = 0, currentV2_last = 0, currentV3_last = 0;
      float currentThreshold = 1000;  //Change to raw values.
      bool reportGPSvalue = true;
      int rx2 = 17;  //New PCB Board , previously 17 from SCG_G107 change to 27.;
      int tx2 = 16;  //New PCB Board , previously 16 from SCG_G107 change to 14.;

//  Sleep
      int sleepMode = 0;
      int sleepPeriod = 0;
      int regSleepMode = 151;
      int regDetectionMode = 152;
      int detectionMode = 0;

//      int runTime = 0;
//      int loadTime = 0;
    //Machine status
      String machState = "Off";
      long tsPowerOff = 0;
      float timeBeforeSleep = 25.0;
    //ADS variables, currently applying default SDA and SCL pins.
      //byte pinSDA = 21;
      //byte pinSCL = 22;
      bool initAds1 = false;
      bool initAds2 = false; 
    //Functions
    //Initialization
      void init(byte delayAtStart, float updateFirmwareCheck, String apTitle, String apName, String apPass, Ticker &ticker, WiFiManager &wm, WiFiClient &wifiClient);    
      void initMqtt(PubSubClient &mqttClient, WiFiManager &wm);   

    //Wi-Fi operations
      void setup_wifi(WiFiManager &wm);
      bool reconnectWifi(WiFiManager &wm);
      void setupWM(Ticker &ticker, WiFiManager &wm, WiFiClient &wifiClient);
      static void WiFiSetupTask(void *pvParameters);      
      void checkResetWiFi(WiFiManager &wm);
      void reportRSSI(WiFiManager &wm, PubSubClient &mqttClient);
      
    //MQTT operations
      void keepAlive(WiFiManager &wm, PubSubClient &mqttClient, WiFiClient &wifiClient);
              
    //Initial State
                             
    //MQTT-related
      bool reconnectMqtt(PubSubClient &mqttClient, WiFiManager &wm);
      bool publishMQTT(PubSubClient &mqttClient, String message, String machStatus, String amount);    
      void callbackMqtt(char* topic, byte* payload, unsigned int length, PubSubClient &mqttClient);
      void resetupMqttClient(PubSubClient &mqttClient);
            
           
    //Miscellaneous operations
      void printToDebug(String printMsg);
      void resetESP(bool checkCond, String reportMessage);
      void checkFirmwareUpdate(WiFiClient &wifiClient, WiFiManager &wm);        
      void EEPROMWriteESP(int currReg, int writeValue);
      void clearSerialBuffer();
      void ISRwdMqtt();
      void ISRwdWifi();
      String lastErrorMessage(int regIndex);
      void watchDog(PubSubClient &mqttClient);
      void reportLastErrors();
      bool isChanged(int changeMode, float &timeLatest, float updatePeriod, double &currValue, double newValue);     
      bool isTimeUpdate(double &timeLatest, float updatePeriod);
      bool software_Reboot();    
      String changeDeviceNumber(int newDevNumber);
      String getDevNumber();
      void setDevNumber(int devNumber);
      void firstTimeSetUp();
      void setEEPROM();
      void resetRelay(int timeDelayMinutes);
      void resetRunTime();
      
      //To set definition of MQTT messages of different pump types.
      String registerDef = "spRegister";
      String registerAlarm = "spAlarm";
      bool timeMinuteCheck(float &timer, float nextUpdate, float firstDelay);
      void resetMCUMemory(int resetMode);
};
#endif
