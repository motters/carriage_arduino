#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>


#pragma once

class communicator
{
public:
	communicator(bool subHub, String ssid_id, String password);
	~communicator(void);
protected:
	String ssid;
	String pass;
	bool sub;
	String nodes;
	String master_ssid;
	String master_pass;

	WiFiServer  _server;
	WiFiClient  _client;

	bool setNodes(String nodes_string);
	bool setSsidPass(String ssid, String pass);
	bool send(String message, WiFiClient curr_client);
	bool waitForClient(WiFiClient curr_client, int max_wait);
	String split(String str, int id);
public:
	String received;

	bool configureMainHub(String nodes);
	bool configureSubHub(void);
	bool sendSubHubData(void);
	bool toHub(String data);
	bool readData();
};

