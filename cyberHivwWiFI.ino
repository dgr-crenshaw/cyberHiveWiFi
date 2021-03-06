/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

/* includes */
#include "Arduino.h"

#include <ESP8266WiFi.h>

#include <DHT.h>

#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT.h>

// Example testing sketch for various DHT humidity/temperature sensors
// See comments above for original author and license
// Modified by dgrc to support MQTT
// Further modified by dgrc to support Node-Red by feeding it JSON data
// Sensor failure handling sends "nan" in json

//sleep setup for void espDeepSleep()
//TODO set back to 1800 (30 minutes) for production
const uint32_t sleepTimeSeconds = 1800; //for sleep time, number of seconds to sleep production
//const uint32_t sleepTimeSeconds = 30; //for sleep time, number of seconds to sleep test
//

//network connection setup for void connectWiFi()
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient clientWiFi;
#include "routerCredentials.h"

//MQTT connection setup for void sendLocalMQTTData()
#include "mqttCredentials.h"

// Set a unique MQTT client ID using the BEEMQTT key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char BEEMQTT_CLIENTID[] = BEEMQTT_KEY __DATE__ __TIME__;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqttLocalBroker(&clientWiFi, BEEMQTT_SERVER,
		BEEMQTT_SERVERPORT, BEEMQTT_CLIENTID, BEEMQTT_USERNAME, BEEMQTT_KEY);

//for JSON character array
Adafruit_MQTT_Publish json = Adafruit_MQTT_Publish(&mqttLocalBroker,
		BEEMQTT_USERNAME "/feeds/json"); //prepping for Node-Red + Mongodb

//MQTT connection setup for void sendLocalMQTTData()
#include "adafruitCredentials.h"

// Set a unique MQTT client ID using the AIOMQTT key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char AIOMQTT_CLIENTID[] = AIOMQTT_KEY __DATE__ __TIME__;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqttAdafruit(&clientWiFi, AIOMQTT_SERVER,
		AIOMQTT_SERVERPORT, AIOMQTT_CLIENTID, AIOMQTT_USERNAME, AIOMQTT_KEY);

// Setup feeds for temperature & humidity & battery
const char TEMPERATURE_01_FEED[] = AIOMQTT_USERNAME "/feeds/tempSensor01";
Adafruit_MQTT_Publish temperature_01 = Adafruit_MQTT_Publish(&mqttAdafruit,
		TEMPERATURE_01_FEED);

const char HUMIDITY_01_FEED[] = AIOMQTT_USERNAME "/feeds/humiditySensor01";
Adafruit_MQTT_Publish humidity_01 = Adafruit_MQTT_Publish(&mqttAdafruit,
		HUMIDITY_01_FEED);

// Setup feeds for additional temperature & humidity feeds
const char TEMPERATURE_02_FEED[] = AIOMQTT_USERNAME "/feeds/tempSensor02";
Adafruit_MQTT_Publish temperature_02 = Adafruit_MQTT_Publish(&mqttAdafruit,
		TEMPERATURE_02_FEED);

const char HUMIDITY_02_FEED[] = AIOMQTT_USERNAME "/feeds/humiditySensor02";
Adafruit_MQTT_Publish humidity_02 = Adafruit_MQTT_Publish(&mqttAdafruit,
		HUMIDITY_02_FEED);

const char TEMPERATURE_03_FEED[] = AIOMQTT_USERNAME "/feeds/tempSensor03";
Adafruit_MQTT_Publish temperature_03 = Adafruit_MQTT_Publish(&mqttAdafruit,
		TEMPERATURE_03_FEED);

const char HUMIDITY_03_FEED[] = AIOMQTT_USERNAME "/feeds/humiditySensor03";
Adafruit_MQTT_Publish humidity_03 = Adafruit_MQTT_Publish(&mqttAdafruit,
		HUMIDITY_03_FEED);

const char BATTERY_FEED[] = AIOMQTT_USERNAME "/feeds/battery";
Adafruit_MQTT_Publish battery = Adafruit_MQTT_Publish(&mqttAdafruit,
		BATTERY_FEED); //prepping for power management

//dht sensor setup for HuzzahESP8266
#define DHTPIN_01 2 //No shield -- 3 channel 1
#define DHTPIN_02 4 //No shield -- 3 channel 2
#define DHTPIN_03 5 //No shield -- 3 channel 3

#define DHTTYPE DHT22
DHT dht_01(DHTPIN_01, DHTTYPE);
DHT dht_02(DHTPIN_02, DHTTYPE); //No shield -- 3 channel
DHT dht_03(DHTPIN_03, DHTTYPE); //No shield -- 3 channel

//make variables global so multiple functions can use their values

//temperature and humidity
float humidityRelative_01;
float humidityRelative_02;
float humidityRelative_03;
float tempFahrenheit_01;
float tempFahrenheit_02;
float tempFahrenheit_03;

//averaging loopLimit battery readings
float sumReads = 0; //initialize variable to sum multiple battery readings prior to averaging
int loopCounter = 0; //initialize loop index
int const loopLimit = 10; //initialize number of iterations of read
float batteryRemaining = 0; //initialize A0 read
float const factorADC = 4.2 / 1024; //give a voltage conversion factor
float averageReadingBattery = 0; //initialize variable to hold averaged value of 10 reads

//converting JSON string to character array
String jsonData;
char jsonPublish[120]; //if this gets too big => runtime crash

void setup() {
	//init serial for debugging
	Serial.begin(115200);

	// inititialize sensor
	dht_01.begin(); //no shield -- channel 1
	dht_02.begin(); //no shield -- channel 2
	dht_03.begin(); //no shield -- channel 3

	//poll the sensors and voltage divider for values
	//then put the board to sleep
	espDeepSleep();

}

void loop() {
	// nothing in the loop since we're using sleep
}

/**************************************
 * functions called in setup
 * outside loop for sleep
 ************************************/

void connectWiFi() {
	// Connect to WiFi access point.

	Serial.println();
	Serial.println();
	delay(10);
	Serial.print(F("Connecting to "));
	Serial.println(WLAN_SSID);

	WiFi.begin(WLAN_SSID, WLAN_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(F("+-"));
	}

	Serial.println();
	Serial.println(F("WiFi connected"));
	Serial.println(F("IP address: "));
	Serial.println(WiFi.localIP());
}

//connect to adafruit mqtt broker
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void adafruitMqttConnect() {
  int8_t ret;

  // Stop if already connected.
  if (mqttAdafruit.connected()) {
    return;
  }

  Serial.print("Connecting to adafruit MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqttAdafruit.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqttAdafruit.connectErrorString(ret));
       Serial.println("Retrying adafruit MQTT connection in 5 seconds...");
       mqttAdafruit.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Adafruit Connected!");
}

//connect to local mqtt broker
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void localBrokerMqttConnect() {
  int8_t ret;

  // Stop if already connected.
  if (mqttLocalBroker.connected()) {
    return;
  }

  Serial.print("Connecting to local broker MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqttLocalBroker.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqttLocalBroker.connectErrorString(ret));
       Serial.println("Retrying local broker MQTT connection in 5 seconds...");
       mqttLocalBroker.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Local Broker Connected!");
}

//separate data collection from data publish local then publish to adafruit.io
void takeReadings() {
	/*****
	 Reading temperature or humidity takes about 250 milliseconds
	 *****/

	humidityRelative_01 = dht_01.readHumidity();
	humidityRelative_02 = dht_02.readHumidity();
	humidityRelative_03 = dht_03.readHumidity();
	tempFahrenheit_01 = dht_01.readTemperature(true);
	tempFahrenheit_02 = dht_02.readTemperature(true);
	tempFahrenheit_03 = dht_03.readTemperature(true);
	Serial.print("Humidity01: ");
	Serial.println(humidityRelative_01);
	Serial.print("Temperature01: ");
	Serial.println(tempFahrenheit_01);
	Serial.print("Humidity02: ");
	Serial.println(humidityRelative_02);
	Serial.print("Temperature02: ");
	Serial.println(tempFahrenheit_02);
	Serial.print("Humidity03: ");
	Serial.println(humidityRelative_03);
	Serial.print("Temperature03: ");
	Serial.println(tempFahrenheit_03);



	//Battery power and monitoring

	//Take readings
	while (loopCounter < loopLimit) {
		batteryRemaining = analogRead(A0); //read raw value 0-1024
		Serial.print("Battery Raw: ");
		Serial.println(batteryRemaining);
		batteryRemaining = batteryRemaining * factorADC; //convert to fraction of 4.2 lipo
		Serial.print("Battery Calibrated: ");
		Serial.println(batteryRemaining);
		sumReads += batteryRemaining; // add to count
		loopCounter++; //increment counter
		delay(100); //
	}

	averageReadingBattery = sumReads / loopLimit; //get the average
	Serial.print("Battery Average: ");
	Serial.println(averageReadingBattery);

	//test all sensors -- publish all successes (fails set to 0.0)
	//TODO clean up failure reporting
	//forcing a string then forcing a "nan" for json is a real kludge

	if ( // battery test
	isnan(averageReadingBattery)) {
		averageReadingBattery = 0.0;
		//String(averageReadingBattery) = "xx"; //forces nan to json
	}

	if ( //dht_01 test
	isnan(tempFahrenheit_01) || isnan(humidityRelative_01)) {
		tempFahrenheit_01 = 0.0;
		//String(tempFahrenheit_01) = "xx"; //forces nan to json
		humidityRelative_01 = 0.0;
		//String(humidityRelative_01) = "xx"; //forces nan to json
	}

	if ( //dht_02 test
	isnan(tempFahrenheit_02) || isnan(humidityRelative_02)) {
		tempFahrenheit_02 = 0.0;
		//String(tempFahrenheit_02)  = "xx"; //forces nan to json
		humidityRelative_02 = 0.0;
		//String(humidityRelative_02)  = "xx"; //forces nan to json
	}

	if ( //dht_03 test
	isnan(tempFahrenheit_03) || isnan(humidityRelative_03)) {
		tempFahrenheit_03 = 0.0;
		//String(tempFahrenheit_03)  = "xx"; //forces nan to json
		humidityRelative_03 = 0.0;
		String(humidityRelative_03) = "xx"; //forces nan to json
	}

	//borrowed with gratitude from http://www.internetoflego.com/weather-station-dht11-mqtt-node-red-google-chart-oh-my/

	// Convert data to JSON string

	jsonData = "{\"data\":{"
			"\"h01\":\"" + String(humidityRelative_01) + "\","
			"\"h02\":\"" + String(humidityRelative_02) + "\","
			"\"h03\":\"" + String(humidityRelative_03) + "\","
			"\"t01\":\"" + String(tempFahrenheit_01) + "\","
			"\"t02\":\"" + String(tempFahrenheit_02) + "\","
			"\"t03\":\"" + String(tempFahrenheit_03) + "\","
			"\"bat\":\"" + String(averageReadingBattery) + "\"}"
			"}";

	Serial.println("----------");
	Serial.println("jsonData: " + jsonData);

	jsonData.toCharArray(jsonPublish, jsonData.length() + 1); //make the string a char array

	Serial.println("jsonDataXformed: " + jsonData);
	Serial.println("----------");
}

//send data
void sendLocalMQTTData() {
	// Wait 10 seconds between measurements.
	delay(10000);

	// ping local mqtt broker a few times to make sure we remain connected
	if (!mqttLocalBroker.ping(3)) {
		// reconnect to local mqtt broker
		if (!mqttLocalBroker.connected())
			localBrokerMqttConnect();
	}

	// Publish JSON character array to MQTT topic
	json.publish(jsonPublish);
	mqttLocalBroker.disconnect(); //force disconnect so other mqtt allowed
	Serial.println("json published");
}

void sendAdafruitIoMQTTData() {
	// Wait 10 seconds between measurements.
	delay(10000);

	// ping local mqtt broker a few times to make sure we remain connected
	if (!mqttAdafruit.ping(3)) {
		// reconnect to local mqtt broker
		if (!mqttAdafruit.connected())
			adafruitMqttConnect();
	}

	// Publish data to adafruit.io
	temperature_01.publish(tempFahrenheit_01);
	humidity_01.publish(humidityRelative_01);
	temperature_02.publish(tempFahrenheit_02);
	humidity_02.publish(humidityRelative_02);
	temperature_03.publish(tempFahrenheit_03);
	humidity_03.publish(humidityRelative_03);
	battery.publish(averageReadingBattery);
	mqttAdafruit.disconnect(); //force disconnect so other mqtt allowed
	Serial.println("adafruit published");
}

//sleep
void espDeepSleep() {
	Serial.println("Wakey Wakey"); //reset jumped to GPIO 16 = D0

	takeReadings(); //get readings from sensors

	connectWiFi();	  //connect to WiFi

	sendLocalMQTTData(); //get and publish data to local broker
	sendAdafruitIoMQTTData(); //get and publish data to adafruit.io

	Serial.println("Nighty Night");

	//go to sleep my little baby
	// deepSleep time is defined in microseconds. Multiply seconds by 1e6
	ESP.deepSleep(sleepTimeSeconds * 1000000);
}
