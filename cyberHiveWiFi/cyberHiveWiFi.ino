// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
// Modified by dgrc to support MQTT adafruit_io dashboard

//TODO pull everything out of loop() and into setup() to support sleep

// DHT 22 sensor parameters
#include "DHT.h"
#define DHTPIN D4 //WeMos DHT22 shield hardwired pin
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// WiFi parameters
#include "ESP8266WiFi.h"
#define WLAN_SSID "NETGEAR75"
#define WLAN_PASS "greenstar582"

//Adafruit IO MQTT parameters
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "dgrc"
#define AIO_KEY "d07a294b8495727687c45fa4f2314ae91fd3fac6"

// MQTT Functions
void connect();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[] PROGMEM = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID,
		MQTT_USERNAME, MQTT_PASSWORD);/****************************** Feeds ***************************************/

// Setup feeds for temperature & humidity & battery
const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt,
		TEMPERATURE_FEED);

const char HUMIDITY_FEED[] PROGMEM = AIO_USERNAME "/feeds/humidity";
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

const char BATTERY_FEED[] PROGMEM = AIO_USERNAME "/feeds/battery";
Adafruit_MQTT_Publish battery = Adafruit_MQTT_Publish(&mqtt, BATTERY_FEED); //prepping for power management

void setup() {
	//init serial
	Serial.begin(115200);

	// init sensor
	dht.begin();

	// Connect to WiFi access point.
	Serial.println();
	Serial.println();
	delay(10);
	Serial.print(F("Connecting to "));
	Serial.println(WLAN_SSID);

	WiFi.begin(WLAN_SSID, WLAN_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(F("."));
	}
	Serial.println();

	Serial.println(F("WiFi connected"));
	Serial.println(F("IP address: "));
	Serial.println(WiFi.localIP());

	// connect to adafruit_io
	connect();
}

void loop() {
	// Wait 10 seconds between measurements.
	delay(10000);

	// ping adafruit io a few times to make sure we remain connected
	if (!mqtt.ping(3)) {
		// reconnect to adafruit_io
		if (!mqtt.connected())
			connect();
	}

	// Reading temperature or humidity takes about 250 milliseconds!
	float humidityRelative = dht.readHumidity();
	float tempFahrenheit = dht.readTemperature(true);
	//prepping for power management
	float batteryRemaining = 50.0;

	// Check if any reads failed and exit early (to try again).
	if (isnan(
			humidityRelative) || isnan(tempFahrenheit) || isnan(batteryRemaining)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	/***
	 Serial.print("Humidity: ");
	 Serial.print(humidityRelative);
	 Serial.print(" %\t");
	 Serial.print(tempFahrenheit);
	 Serial.print(" *F\t");
	 ***/

	// Publish data
	if (!battery.publish(batteryRemaining)) //prepping for power management
		Serial.println(F("Failed to publish battery charge"));
	else
		Serial.println(F("Battery charge published!"));

	if (!temperature.publish(tempFahrenheit))
		Serial.println(F("Failed to publish temperature"));
	else
		Serial.println(F("Temperature published!"));

	if (!humidity.publish(humidityRelative))
		Serial.println(F("Failed to publish humidity"));
	else
		Serial.println(F("Humidity published!"));
	Serial.println(humidityRelative);

}

// connect to adafruit_io via MQTT
void connect() {

	Serial.print(F("Connecting to Adafruit IO... "));
	int8_t ret;

	while ((ret = mqtt.connect()) != 0) {
		switch (ret) {
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
		}

		if (ret >= 0)
			mqtt.disconnect();
		Serial.println(F("Retrying connection..."));
		delay(5000);
	}

	Serial.println(F("Adafruit IO Connected!"));
}
