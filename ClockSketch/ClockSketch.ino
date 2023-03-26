#include "SevSeg.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>  
#include <TimeLib.h>



//Set time offset here
int timeOffset = 7200; //in seconds (summertime)
//int timeOffset = 3600; //in seconds (wintertime)

const char *ssid     = "enter your wlan name";
const char *password = "and your wlan password";



SevSeg sevseg;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset, 60000);


String timeStr = "0000";
byte last_minute, minute_, hour_;
byte first_minute;
int oldTime = -999999;
byte numDigits = 4; 
byte digitPins[] = {22, 15, 13, 12}; //digit 1,2,3,4
byte segmentPins[] = {5, 0, 26, 33, 32, 17, 27,25}; //segment a,b,c,d,e,f,g,dp

int timeInt = 0;
bool showDot = true;
TaskHandle_t Task1;
TaskHandle_t Task2;

unsigned long unix_epoch;

void setup() {
  //Serial.begin(115200);
  WiFi.begin(ssid, password);
  timeClient.begin();
  timeClient.setTimeOffset(timeOffset);
  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(100); 
  
  while ( WiFi.status() != WL_CONNECTED ) {delay(500);}

  timeClient.update();
  unix_epoch = timeClient.getEpochTime();
  
  minute_ = minute(unix_epoch);
  hour_   = hour(unix_epoch);
  
  last_minute = minute_;
  first_minute = minute_;
  
  timeStr[3]  = minute_ % 10 + 48;
  timeStr[2]  = minute_ / 10 + 48;
  timeStr[1]  = hour_   % 10 + 48;
  timeStr[0]  = hour_   / 10 + 48;
  sevseg.setNumber(timeStr.toInt(),2);

  //(Task function, name, stack size, parameter, prio, task handle pointer to keep track, core)
  xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,1,&Task1,0); //Task on Core 0 with prio 1 
  delay(500); 
  xTaskCreatePinnedToCore(Task2code,"Task2",10000,NULL,1,&Task2,1); //Task on Core 1 with prio 1 
  delay(500); 
}

void Task1code( void * pvParameters ){for(;;) {sevseg.refreshDisplay();delay(1);}}

void Task2code( void * pvParameters ){
  for(;;){
    delay(200);
    if (millis()-oldTime > 59000 || first_minute == minute_) {
      timeClient.update();
      unix_epoch = timeClient.getEpochTime();
      minute_ = minute(unix_epoch);
      if (last_minute != minute_) {
        oldTime = millis();
        minute_ = minute(unix_epoch);
        last_minute = minute_;
        hour_   = hour(unix_epoch);
        timeStr[3]  = minute_ % 10 + 48;
        timeStr[2]  = minute_ / 10 + 48;
        timeStr[1]  = hour_   % 10 + 48;
        timeStr[0]  = hour_   / 10 + 48;
        timeInt = timeStr.toInt();
        sevseg.setNumber(timeStr.toInt(),2);
      }
    }
  }
}

void loop(){}
