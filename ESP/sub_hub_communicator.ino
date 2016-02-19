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
const char *ssid = "SUBHUB-API-KEY";
const char *password = "SUBHUB-PASSWORD";
bool main_hub = false;

/**
 * Misc global vars
 */ 
communicator* nodeCommunicator = new communicator(true, String(ssid), String(password));



/** 
 * Setup the device
 */
void setup() {
	// Setup serial and allow device to settle
	delay(1000);
	Serial.begin(115200);
}


/** 
 * Setup the device
 */
void loop() {

	// If the main hub connection details are not collected then handle clients
	if(!main_hub){
		// Wait for main hub
		if(nodeCommunicator->configureSubHub())
			main_hub = true;
	}

	// If main hub has been defined
	if(main_hub){
		// Wait for serial to be sent by sub hub (ardunio)
		if(Serial.available() > 0){
			// Read the data ready to be sent to main hub
			String data = Serial.readString();
			// Send the data
			if(!nodeCommunicator->toHub(data)){
				// There was an error do something like re-try
			}else{
                                Serial.println("sent!");
                        }
		}
	}
}

