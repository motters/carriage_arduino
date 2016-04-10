
/**
 * Include libs
 */
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>


/**
 * Setup ethernet connection
 *
 * @author Sam Mottley
 */
// Set mac address
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x19, 0x28 };
// Set the web panel ip
char server[] = "192.168.0.171"; // 192.168.0.171
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 135, 1);
// Initialize the Ethernet client library
EthernetClient client;


/**
 * Hub unique information
 *
 * @author Sam Mottley
 */
const char username[] = "hub_225468";
const char enc_key[] = "tzGN3ntqyWSIZML6nYYHeB2EpDk2IH92"; 
char password[] = "password";
const char api_key[] = "8zA4N3vDrhgEFQugX4ThtO1Ch";
String serial_config = "";



/**
 * Configure upload interval TEMP
 *
 * @author Sam Mottley
 */
unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 100L * 500L; // delay between updates, in milliseconds
boolean connected_to = false;
//String data = "";
char *cstr;
//constexpr const int* addr(const int& ir) {
//	return &ir;
//}


/**
 * Setup the application
 *
 * @author Sam Mottley
 */
void setup()
{
	// Open serial 115200 and wait for port to open:
	Serial.begin(115200);
	Serial1.begin(115200);
	
	// Start the Ethernet connection:
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP tring static IP instead.");
		// Try to congifure using IP address instead of DHCP:
		Ethernet.begin(mac, ip);
	}

	// Get the hubs and associated sub hubs configuration
	while(serial_config == ""){
		serial_config = httpRequestConfig();
	}
	cstr = new char[serial_config.length() + 1];
	strcpy(cstr, serial_config.c_str());

}


/**
 * Main program loop
 *
 * @author Sam Mottley
 */
void loop()
{
	// Set data to send to sub hubs    
	String Hubs = "[{\"s\": \""+String(api_key)+"\", \"p\":\""+String(password)+"\"}, ";
	String SubHubs = cstr; SubHubs.remove(0,1);  SubHubs = Hubs + SubHubs; // EG   [{"s": "8zA4N3vDrhgEFQugX4ThtO1Ch", "p":"password"}, {"s":"fweifweiojfiojoi","p":"ioj","m":[{"t":"3","in":"500","mc":"okpok"}]},{"s":"sub_v1_1","p":"password","m":[{"t":"2","in":"8000","mc":"SH-D-3"},{"t":"2","in":"5000","mc":"SH-I2C-105"}]}]

	// Send data to wirless module
	Serial1.println(SubHubs);
	Serial.println(SubHubs);

	// Variable to store incomming information
	String data = "";

	// Loop forever now config settings are loaded
	while (true)
	{
		// Check for incomming requests
		if(Serial1.available() > 0)
		{
			// Read the data
			data = Serial1.readString();

			// Remove line feed and return 
			data.replace("\n","");
			data.replace("\r","");

			// Print data to serial 0 to debug
			Serial.println(data);

			// Upload data to web sever
			boolean uploaded = false;
			while(!uploaded){
				uploaded = httpRequestUpload(data);
				if(!uploaded)
					delay(500);
			}

			// Clear data from memory
			data = "";
		}
	}

}



/**
 * Upload the data from the module buffer to the web server
 *
 */
boolean httpRequestUpload(String data_string)
{
	// Ensure socket is not in use
	if (!client.connected()) {
		client.stop();
	}

	// When socket free open a connection       Example Data String: sub_v1_1}SH-D-3!2!1459533399@24.2:12.5,1459533542@25.2:13.5,1459533642@27.2:13.5,1459533742@15.2:24.5}
	if (client.connect(server, 80)) {
		// Make the http request to server     ?data="+data_string+"
		client.println("POST /api/v1/upload/" + String(api_key) + "?data="+data_string+" HTTP/1.1\nHost: 192.168.0.171\napi: " + String(api_key) + "\nenc: " + String(enc_key) + "\nusername: " + String(username) + "\npassword: " + String(password) + "\nContent-Type: multipart/form-data;\nConnection: close");
		client.println();
	}

	// Wait for server to recieve request
	while (client.connected() && !client.available()) delay(1);

	// Retrieve data from server and store in buffer
	String data = "";
	while (client.connected() || client.available()) {
		char c = client.read();
		//Serial.print(c);
		if (c == '\n') {
			data += ' ';
		} else {
			data += c;
		}
	}

	// Close the connection / socket
	client.stop();

	return true;
}


/**
 * Retreive the hubs and sub hub configuration from server
 *
 * @author Sam Mottley
 */
String httpRequestConfig()
{
	String data = "";
	// Ensure socket is not in use
	if (!client.connected()) {
		client.stop();
	}

	// When socket free open a connection
	if (client.connect(server, 80)) {
		// Make the http request to server
		client.println("GET /api/v1/config/" + String(api_key) + " HTTP/1.1\nHost: 192.168.0.171\napi: " + String(api_key) + "\nenc: " + String(enc_key) + "\nusername: " + String(username) + "\npassword: " + String(password) + "\nConnection: close");
		//client.println("GET /test HTTP/1.1\nHost: 192.168.33.17\napi: "+String(api_key)+"\nenc: "+String(enc_key)+"\nusername: "+String(username)+"\npassword: "+String(password)+"\nConnection: close");
		client.println();
	}

	// Wait for server to recieve request
	while (client.connected() && !client.available()) delay(1);

	// Retrieve data from server and store in buffer
	while (client.connected() || client.available()) {
		char c = client.read();
		//Serial.print(c);
		if (c == '\n') {
			data += ' ';
		} else {
			data += c;
		}
	}

	// Close the connection / socket
	client.stop();

	// Take only the content not the headers
	const char *PATTERN1 = " [{\"s\":\"";
	const char *PATTERN2 = " 0";

	char *target = NULL;
	char *start, *end;
	const char * s = data.c_str();
	if ( start = strstr( s, PATTERN1 ) )
	{
		start += strlen( PATTERN1 );
		if ( end = strstr( start, PATTERN2 ) )
		{
			target = ( char * )malloc( end - start + 1 );
			memcpy( target, start, end - start );
			target[end - start] = '\0';
		}else{
			return "";
		}
	}else{
		return "";
	}

	// Format the data and ensure valid json
	data = "[{\"s\":\"" + String(target);
	free( target );
	return data;
}


