/**
 * Include libraries
 */
//#include <StandardCplusplus.h>
#include <communicator.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>



/** 
 * Set the main hub wireless settings
 * this will be printed on the side of the device and unique to each sub hub
 */
const char *ssid = "MAINHUB_SSID";
const char *password = "MAINHUB_PASSWORD";


/**
 * Misc global vars
 */ 
// [{"s": "MAINHUB_SSID", "p":"MAINHUB_PASSWORD"}, {"s": "SUBHUB-API-KEY", "p": "SUBHUB-PASSWORD"}]
String connection_data_serial = "[{\"s\": \"MAINHUB_SSID\", \"p\":\"MAINHUB_PASSWORD\"}, {\"s\": \"SUBHUB-API-KEY\", \"p\": \"SUBHUB-PASSWORD\"}]"; // , {\"s\": \"CARRIAGEHUB_2\", \"p\": \"CARRIAGEHUB_PASSWORD\"}
communicator* nodeCommunicator = new communicator(false, String(ssid), String(password));
unsigned long configSubHubInterval = 500000;
unsigned long configSubHubPrevious = 0;


/** 
 * Setup the device
 */
void setup() {
	// Setup serial and allow device to settle
	delay(1000);
	Serial.begin(115200);
	

	// Set node string
	setNodeString();

	// Send sub hub the connection data 
        bool configed = false;
        while(!configed){
	        configed = nodeCommunicator->sendSubHubData();
                delay(10000);
        }
}


/** 
 * Setup the device
 */
void loop() {
	// Wait for data to be sent 
	if(nodeCommunicator->readData()){
		// Print the data to serial for the ardunio to uploads via ethernet
		Serial.println(nodeCommunicator->received);
	}

	// Every 30 minutes re send the ssid and password incase any sub hub was added or reset
	unsigned long currentMillis = millis();
	if (currentMillis - configSubHubPrevious >= configSubHubInterval) {
		// Run configuration
		nodeCommunicator->configureMainHub(connection_data_serial);
		// Save last time configuation was ran
		configSubHubPrevious = currentMillis;
	}
}


/** 
 * Read the node data string from ardunio
 */
void setNodeString() {
	// Wait for the ardunio to send the connection data
	bool wait = false;
	while (!wait) {
		if (Serial.available() > 0) {
			// Set the string
			connection_data_serial = Serial.readString();
			// Configure the class
			wait = nodeCommunicator->configureMainHub(connection_data_serial);
      		}
	}
        Serial.println("configured");
}
