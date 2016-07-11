#include "Arduino.h"
// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
// Modified by dgrc to support MQTT
// Further modified by dgrc to support Node-Red by feeding it JSON data

// DHT 22 sensor parameters
#include "DHT.h"

//for three channel, use D1 D2 D3

/* for WeMos D1
 #define DHTPIN_01 D4 //WeMos DHT22 shield hardwired pin
 //#define DHTPIN_01 D1 //No shield -- 3 channel
 #define DHTPIN_02 D2 //No shield -- 3 channel
 #define DHTPIN_03 D3 //No shield -- 3 channel
 */

//for HuzzahESP8266
//#define DHTPIN_01 D1 //WeMos DHT22 shield hardwired pin
#define DHTPIN_01 2 //No shield -- 3 channel
#define DHTPIN_02 4 //No shield -- 3 channel
#define DHTPIN_03 5 //No shield -- 3 channel

#define DHTTYPE DHT22
DHT dht_01(DHTPIN_01, DHTTYPE);
DHT dht_02(DHTPIN_02, DHTTYPE); //No shield -- 3 channel
DHT dht_03(DHTPIN_03, DHTTYPE); //No shield -- 3 channel

// WiFi parameters
#include "ESP8266WiFi.h"

#include "routerCredentials.h"

const uint32_t sleepTimeSeconds = 3600; //for sleep time, number of seconds to sleep

//Adafruit IO MQTT parameters
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "mqttCredentials.h"

#define BEEMQTT_SERVER "192.168.1.11"
#define BEEMQTT_SERVERPORT 1883

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient clientWiFi;

// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[] PROGMEM = BEEMQTT_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM = BEEMQTT_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM = BEEMQTT_USERNAME;
const char MQTT_PASSWORD[] PROGMEM = BEEMQTT_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&clientWiFi, MQTT_SERVER, BEEMQTT_SERVERPORT,
		MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

// Setup feeds for temperature & humidity & battery
const char TEMPERATURE_01_FEED[] PROGMEM
= BEEMQTT_USERNAME "/feeds/tempSensor01";
Adafruit_MQTT_Publish temperature_01 = Adafruit_MQTT_Publish(&mqtt,
		TEMPERATURE_01_FEED);

const char HUMIDITY_01_FEED[] PROGMEM
= BEEMQTT_USERNAME "/feeds/humiditySensor01";
Adafruit_MQTT_Publish humidity_01 = Adafruit_MQTT_Publish(&mqtt,
		HUMIDITY_01_FEED);

// Setup feeds for additional temperature & humidity feeds
const char TEMPERATURE_02_FEED[] PROGMEM
= BEEMQTT_USERNAME "/feeds/tempSensor02";
Adafruit_MQTT_Publish temperature_02 = Adafruit_MQTT_Publish(&mqtt,
		TEMPERATURE_02_FEED);

const char HUMIDITY_02_FEED[] PROGMEM
= BEEMQTT_USERNAME "/feeds/humiditySensor02";
Adafruit_MQTT_Publish humidity_02 = Adafruit_MQTT_Publish(&mqtt,
		HUMIDITY_02_FEED);

const char TEMPERATURE_03_FEED[] PROGMEM
= BEEMQTT_USERNAME "/feeds/tempSensor03";
Adafruit_MQTT_Publish temperature_03 = Adafruit_MQTT_Publish(&mqtt,
		TEMPERATURE_03_FEED);

const char HUMIDITY_03_FEED[] PROGMEM
= BEEMQTT_USERNAME "/feeds/humiditySensor03";
Adafruit_MQTT_Publish humidity_03 = Adafruit_MQTT_Publish(&mqtt,
		HUMIDITY_03_FEED);

const char BATTERY_FEED[] PROGMEM = BEEMQTT_USERNAME "/feeds/battery";
Adafruit_MQTT_Publish battery = Adafruit_MQTT_Publish(&mqtt, BATTERY_FEED); //prepping for power management

//for JSON character array
const char JSON_FEED[] PROGMEM = BEEMQTT_USERNAME "/feeds/json";
Adafruit_MQTT_Publish json = Adafruit_MQTT_Publish(&mqtt, JSON_FEED); //prepping for Node-Red + Mongodb

void setup() {
	//init serial
	//Serial.begin(115200);

	// init sensor
	dht_01.begin();
	dht_02.begin(), //No shield -- 3 channel
	dht_03.begin(), //No shield -- 3 channel
	//connect to WiFi
	connectWiFi();

	//connect to local mqtt server
	connectMQTT();

	espDeepSleep();

}

void loop() {
//empty loop to support sleep mode
}

void connectWiFi() {
	// Connect to WiFi access point.

	//Serial.println();
	//Serial.println();
	delay(10);
	//Serial.print(F("Connecting to "));
	//Serial.println(WLAN_SSID);

	WiFi.begin(WLAN_SSID, WLAN_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		//Serial.print(F("+-"));
	}

	//Serial.println();
	//Serial.println(F("WiFi connected"));
	//Serial.println(F("IP address: "));
	//Serial.println(WiFi.localIP());

}

// connect to local mqtt server via MQTT
void connectMQTT() {

	//Serial.print(F("Connecting to local mqtt server... "));
	int8_t ret;

	while ((ret = mqtt.connect()) != 0) {

		/*switch (ret) {
		 case 1:
		 Serial.println(F("Wrong protocol"));
		 break;
		 case 2:
		 Serial.println(F("ID rejected"));
		 break;
		 case 3:
		 Serial.println(F("Server unavailable"));
		 break;
		 case 4:
		 Serial.println(F("Bad user/pass"));
		 break;
		 case 5:
		 Serial.println(F("Not authorized"));
		 break;
		 case 6:
		 Serial.println(F("Failed to subscribe"));
		 break;
		 default:
		 Serial.println(F("Connection failed"));
		 break;
		 }*/

		if (ret >= 0)
			mqtt.disconnect();
		//Serial.println(F("Retrying connection..."));
		delay(5000);
	}

	//Serial.println(F("local mqtt server Connected!"));
}

void sendMQTTData() {
	// Wait 10 seconds between measurements.
	delay(10000);

	// ping local mqtt server a few times to make sure we remain connected
	if (!mqtt.ping(3)) {
		// reconnect to local mqtt server
		if (!mqtt.connected())
			connectMQTT();
	}

	/*****
	 Reading temperature or humidity takes about 250 milliseconds
	 *****/

	float humidityRelative_01 = dht_01.readHumidity();
	float humidityRelative_02 = dht_02.readHumidity();
	float humidityRelative_03 = dht_03.readHumidity();
	float tempFahrenheit_01 = dht_01.readTemperature(true);
	float tempFahrenheit_02 = dht_02.readTemperature(true);
	float tempFahrenheit_03 = dht_03.readTemperature(true);

	//Battery power and monitoring

	float sumReads = 0; //initialize variable to sum multiple battery readings prior to averaging
	int loopCounter = 0; //initialize loop index
	int const loopLimit = 10; //initialize number of iterations of read
	float batteryRemaining = 0; //initialize A0 read
	float const factorADC = 4.2 / 1024; //give a voltage conversion factor
	float averageReadingBattery = 0; //initialize variable to hold averaged value of 10 reads

//Take readings
	while (loopCounter < loopLimit) {
		batteryRemaining = analogRead(A0); //read raw value 0-1024
		//Serial.print("Battery Raw: ");
		//Serial.println(batteryRemaining);
		batteryRemaining = batteryRemaining * factorADC; //convert to fraction of 4.2 lipo
		//Serial.print("Battery Calibrated: ");
		//Serial.println(batteryRemaining);
		sumReads += batteryRemaining; // add to count
		loopCounter++; //increment counter
		delay(100); //
	}

	averageReadingBattery = sumReads / loopLimit; //get the average
	//Serial.print("Battery Average: ");
	//Serial.println(averageReadingBattery);

	//test all sensors -- publish all successes (fails set to 0.0)
	//TODO clean up failure reporting

	if ( // battery test
	isnan(averageReadingBattery)) {
		averageReadingBattery = 0.0;
	}
	// Publish battery data
	battery.publish(averageReadingBattery);
	//Serial.println(averageReadingBattery);

	if ( //dht_01 test
	isnan(tempFahrenheit_01) || isnan(humidityRelative_01)) {
		tempFahrenheit_01 = 0.0;
		humidityRelative_01 = 0.0;
	}
	temperature_01.publish(tempFahrenheit_01);
	humidity_01.publish(humidityRelative_01);

	if ( //dht_02 test
	isnan(tempFahrenheit_02) || isnan(humidityRelative_02)) {
		tempFahrenheit_02 = 0.0;
		humidityRelative_02 = 0.0;
	}
	temperature_02.publish(tempFahrenheit_02);
	humidity_02.publish(humidityRelative_02);

	if ( //dht_03 test
	isnan(tempFahrenheit_03) || isnan(humidityRelative_03)) {
		tempFahrenheit_03 = 0.0;
		humidityRelative_03 = 0.0;
	}
	temperature_03.publish(tempFahrenheit_03);
	humidity_03.publish(humidityRelative_03);

	//borrowed with gratitude from http://www.internetoflego.com/weather-station-dht11-mqtt-node-red-google-chart-oh-my/

	// Convert data to JSON string

	String jsonData = "{\"data\":{"
			"\"h01\":\"" + String(humidityRelative_01) + "\","
			"\"h02\":\"" + String(humidityRelative_02) + "\","
			"\"h03\":\"" + String(humidityRelative_03) + "\","
			"\"t01\":\"" + String(tempFahrenheit_01) + "\","
			"\"t02\":\"" + String(tempFahrenheit_02) + "\","
			"\"t03\":\"" + String(tempFahrenheit_03) + "\","
			"\"b\":\"" + String(averageReadingBattery) + "\"}"
			"}";

//Serial.println("----------");
//Serial.println("jsonData: " + jsonData);

	// Convert JSON string to character array
	char jsonPublish[120]; //if this gets too big => runtime crash

	jsonData.toCharArray(jsonPublish, jsonData.length() + 1); //make the string a char array

//Serial.println("jsonDataXformed: " + jsonData);
//Serial.println("----------");

	// Publish JSON character array to MQTT topic
	json.publish(jsonPublish);
}

void espDeepSleep() {
	//Serial.println("Wakey Wakey"); //reset jumped to GPIO 16 = D0
	sendMQTTData(); //get and publish data
	//Serial.println("Nighty Night");

	//go to sleep my little baby
	// deepSleep time is defined in microseconds. Multiply seconds by 1e6
	ESP.deepSleep(sleepTimeSeconds * 1000000);
}
