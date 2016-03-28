#include "communicator.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define SERVER_IP_ADDR			"192.168.4.1"
#define SERVER_PORT				4011

/**
 * Init the class and usage
 *
 * @author Sam Mottley
 */
communicator::communicator(bool subHub, String ssid_id, String password): 
	_server(SERVER_PORT)
{ 
	// Set whether the wirless is the main hub or sub hub
	this->sub = subHub;

	// Set the info
	this->setSsidPass(ssid_id, password); 

	// Set the wifi mode and turn on access point
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP( ssid_id.c_str(), password.c_str() );

	// begin the server
  	this->_server.begin();
}

/**
 * Destructor
 */
communicator::~communicator(void){ }

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
String communicator::split(String in_str, int id) {

	char sz[in_str.length()+1];
    in_str.toCharArray(sz, in_str.length()+1); 

	char *p = sz;
	char *str;
	int id_c = 0;
	while ((str = strtok_r(p, ",", &p)) != NULL){
		if(id_c == id){
			return String(str);
		}else{
			id_c++;
		}
	}
	return "0";
}

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::setSsidPass(String ssid_in, String pass_in)
{
	ssid = ssid_in;
	pass = pass_in;

	return true;
}

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::configureMainHub(String nodes_string)
{
	// decode the node adresses
	this->setNodes(nodes_string);

	// configure sub hub data
	this->sendSubHubData();
}

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::setNodes(String nodes_string)
{
	nodes = nodes_string;
	// De code system settings (dynamic buffer size still needs to be sorted)
	//DynamicJsonBuffer jsonBuffer; // Use this for now but should be static to use less memory
	//this->nodes = jsonBuffer.parseArray(nodes_string);
}

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::sendSubHubData(void)
{
	DynamicJsonBuffer jsonBuffer; // Use this for now but should be static to use less memory
	//::Serial.println(this->nodes);
	JsonArray& connection_data = jsonBuffer.parseArray(this->nodes);

	// Send data to sub hubs
	for (int i = 0; i < (sizeof(connection_data) / sizeof(int)); i++) {
		// ignore zero
		if (i == 0)
		  continue;

		//Change mode
		WiFi.mode(WIFI_STA);
		delay(200);

		// Connect to client
		WiFiClient curr_client;
		WiFi.begin(connection_data[i]["s"], connection_data[i]["p"]);

		// Check status
		int wait = 2500;
		while((WiFi.status() == WL_DISCONNECTED) && wait--)
			delay(3);

		// If the connection timed out
		if (WiFi.status() != WL_CONNECTED){ //3
			//::Serial.println("status 3 return false");
			WiFi.mode(WIFI_AP_STA);
			continue;
			//return false;
		}

		// Connect to the node's server
		if (!curr_client.connect(SERVER_IP_ADDR, SERVER_PORT)) {
			//::Serial.println("cant connect to node continue");
			WiFi.mode(WIFI_AP_STA);
			continue;
			//return false;
		}

		// Send main hubs ssid and password
		if (!this->send(this->ssid+","+this->pass, curr_client)){
			WiFi.mode(WIFI_AP_STA);
			//::Serial.println("cant send data conintue");
			continue;
			//return false;
		}

		// Disconnect from client
		curr_client.stop();
		WiFi.disconnect();
		//::Serial.println("disconnected");

		//Change mode 
		WiFi.mode(WIFI_AP_STA);
		delay(200);
	}

	// Memory leak some where :s
	delete jsonBuffer; delete connection_data;

	return true;
}

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::send(String message, WiFiClient curr_client)
{
	curr_client.println( message.c_str() );

	if (!this->waitForClient(curr_client, 1000)){
		//::Serial.println("cant connect to client returning false");
		return false;
	}

	String response = curr_client.readStringUntil('\r');
	curr_client.readStringUntil('\n');

	/*if (response.length() <= 2){ 
		::Serial.println("length greating than two returning false");
		return false;
	}*/

	this->received = response;

	return true;
}

/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::readData()
{
	while (true) {
		this->_client = this->_server.available();
		if (!this->_client){
			//break;
			//::Serial.println("No client");
			return false;
		}

		if (!this->waitForClient(_client, 3000)) {
			//::Serial.println("Waiting for client");
			//continue;
			continue;
		}

		// Read in request and pass it to the supplied handler
		this->received = this->_client.readStringUntil('\r');
		this->_client.readStringUntil('\n');

		//::Serial.println(this->received);

		// Send the response back to the client of success
		if (this->_client.connected())
			this->_client.println("200");

		return true;
	}
	
	return false;
}


/**
 * PRIVATE: 
 *
 * @author Sam Mottley
 */
bool communicator::waitForClient(WiFiClient curr_client, int max_wait)
{
	int wait = max_wait;
	while(curr_client.connected() && !curr_client.available() && wait--)
		delay(3);

	/* Return false if the client isn't ready to communicate */
	/*if (WiFi.status() == WL_DISCONNECTED || !curr_client.connected()){
		::Serial.println("client is not ready to communicate returning false");
		return false;
	}*/
	
	return true;
}


/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::configureSubHub(void)
{
	// Wait for main hub to send its conenction details
	bool mainhub = false;
	while(!mainhub)
	{
		// Read any data sent
		if(this->readData()){
			//set the main hub data
			this->master_ssid = this->split(this->received, 0);
			this->master_pass = this->split(this->received, 1);
			//::Serial.println(master_ssid);
			//::Serial.println(master_pass);
			
			// Exit the while loop
			mainhub = true;
		}
	}

	return true;
}


/**
 * PUBLIC: 
 *
 * @author Sam Mottley
 */
bool communicator::toHub(String data)
{
	// Type conversions (bad coding lol)
	char ssid_char[this->master_ssid.length()+1]; char pass_char[this->master_pass.length()+1];
    this->master_ssid.toCharArray(ssid_char, this->master_ssid.length()+1); 
	this->master_pass.toCharArray(pass_char, this->master_pass.length()+1);

	//Change mode
	WiFi.mode(WIFI_STA);
	delay(200);

	// Connect to client
	WiFiClient curr_client;
	WiFi.begin(ssid_char, pass_char);
	//::Serial.println(ssid_char);
	//::Serial.println(pass_char);

	// Check status
	int wait = 2500;
	while((WiFi.status() == WL_DISCONNECTED) && wait--)
		delay(3);

	// If the connection timed out
	if (WiFi.status() != 3){
		//::Serial.println("Status issue in connecting to node");
		WiFi.mode(WIFI_AP_STA);
		return false;
	}

	// Connect to the node's server
	if (!curr_client.connect(SERVER_IP_ADDR, SERVER_PORT)){ 
		//::Serial.println("cant't connect to node server returning false");
		WiFi.mode(WIFI_AP_STA);
		return false;
	}

	// Send main hubs ssid and password
	if (!this->send(data, curr_client)){
		//::Serial.println("could not send data returing false");
		WiFi.mode(WIFI_AP_STA);
		return false;
	}

	// Disconnect from client
	curr_client.stop();
	WiFi.disconnect();

	//Change mode 
	WiFi.mode(WIFI_AP_STA);
	delay(200);

	return true;
}
