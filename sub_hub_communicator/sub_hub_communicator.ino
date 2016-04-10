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


/**
 * Misc global vars
 */ 
communicator* nodeCommunicator = new communicator(true, String(ssid), String(password)); // Communicator class
String data_buffer = ""; // Buffer to hold data
int data_buffer_length = 1000; // Allowed length of data buffer
bool main_hub = false; // Has the sub hub been configured by main hub?


/** 
 * Setup the device
 */
void setup()
{
	// Setup serial and allow device to settle
	delay(1000);
	Serial.begin(115200);
    //Serial.setDebugOutput(true);
}


/** 
 * Setup the device
 */
void loop() 
{
	// If the main hub connection details are not collected then handle clients
	if(!main_hub)
	{
		// Wait for main hub
		if(nodeCommunicator->configureSubHub())
		{
			main_hub = true;  
            //Serial.println("connection_details_recieved");
        }
	}

	// If main hub has been defined
	if(main_hub)
	{
		// Wait for serial to be sent by sub hub (ardunio)
		if(Serial.available() > 0)
		{
			// Read the data
			String data = Serial.readString();

			// Add new data to buffer
			data_buffer += data;

			//Reply
			//Serial.println("data_recieved");

			// Should the buffer be sent to the main hub?
            if(data_buffer.length() >= data_buffer_length)
			{
				// Send the data
				bool sent = false;
				while(!sent)
				{
					sent = nodeCommunicator->toHub(String(ssid)+"}"+data_buffer);
					if(!sent)
						delay(1000);
				}

				// Clear buffer
				data_buffer = "";

				// Reply
				//Serial.println("data_sent");
			}			
		}
	}
}

