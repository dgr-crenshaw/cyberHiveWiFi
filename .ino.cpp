#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2016-10-02 21:16:40

#include "Arduino.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT.h>
#include "routerCredentials.h"
#include "mqttCredentials.h"
#include "adafruitCredentials.h"
void setup() ;
void loop() ;
void connectWiFi() ;
void connectMQTT(int brokerToCall) ;
void takeReadings() ;
void sendLocalMQTTData() ;
void sendAdafruitIoMQTTData() ;
void espDeepSleep() ;

#include "cyberHivwWiFI.ino"


#endif
