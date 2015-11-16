
/**
 * Include libs
 */
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <AESLib.h>
//#include <ESP8266_TCP.h>
//#include <SoftwareSerial.h>
//SoftwareSerial wirless(1, 0);

// ESP8266 Class
//ESP8266_TCP wifi;

// Target Access Point
#define ssid         "ESP_AP"
#define pass         "123456789"

// Connect this pin to CH_PD pin on ESP8266
#define PIN_RESET    6

/**
 * Setup ethernet connection
 *
 * @author Sam Mottley
 */
// Set mac address
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x19, 0x28 };
// Set the web panel ip
char server[] = "192.168.0.171";
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 135, 1);
// Initialize the Ethernet client library
EthernetClient client;


/**
 * Hub unique information
 *
 * @author Sam Mottley
 */
//const uint8_t key[] = {'t','z','G','N','3','n','t','q','y','W','S','I','Z','M','L','6','n','Y','Y','H','e','B','2','E','p','D','k','2','I','H','9','2'};
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
constexpr const int* addr(const int& ir) {
  return &ir;
}


/**
 * Setup the application
 *
 * @author Sam Mottley
 */
void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  //wirless.begin(115200);

  // Start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP tring static IP instead.");
    // Try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  // Get the hubs and associated sub hubs configuration
  serial_config = httpRequestConfig();
  cstr = new char[serial_config.length() + 1];
  strcpy(cstr, serial_config.c_str());

  /*wifi.begin(&wirless, &Serial, PIN_RESET);

  // Check that ESP8266 is available
  if (wifi.test())
  {
    // Connect to target Access Point
    String ip = connectAP();
    Serial.print(ip);
    // Open TCP Server on port 2000 and 30 seconds for connection timeout (Max 2880)
    wifi.openTCPServer(2000, 30);
  } else {
    Serial.print("nope");
  }*/
}


/**
 * Main program loop
 *
 * @author Sam Mottley
 */
void loop()
{
  // De code system settings (dynamic buffer size still needs to be sorted)
  Serial.println(serial_config);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(cstr);
  const char* sub_hub_demo_api = root["sub_hubs"][0]["api_key"];

  // Loop forever now config settings are loaded
  while (true)
  {
    // temp
    Serial.println(sub_hub_demo_api); delay(5000);

    /*
    int dataState = wifi.isNewDataComing(WIFI_SERVER);
    if (dataState != WIFI_NEW_NONE) {
      if (dataState == WIFI_NEW_CONNECTED) {
        // Connected with TCP Client Side
        Serial.println("Status : Connected");
      } else if (dataState == WIFI_NEW_DISCONNECTED) {
        // Disconnected from TCP Client Side
        Serial.println("Status : Disconnected");
      } else if (dataState == WIFI_NEW_MESSAGE) {
        // Got a message from TCP Client Side
        Serial.println("ID : " + String(wifi.getId()));
        Serial.println("Message : " + wifi.getMessage());
        wifi.closeTCPConnection(0);
      } else if (dataState == WIFI_NEW_SEND_OK) {
        // Message transfer has successful
        Serial.println("SENT!!!!");
      }
    }*/


    // Check for incomming requests
    // Check sub hub is in the config array
    // proive the sub hubs configuration OR save the module data from sub hub to buffer

    // Check the size of the module data buffer
    // if greated than x then upload the buffered module data to the web server

  }

}


/**
 * Upload the data from the module buffer to the web server
 */
boolean httpRequestUpload()
{

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
    client.println("GET /api/v1/config/" + String(api_key) + " HTTP/1.1\nHost: 192.168.33.17\napi: " + String(api_key) + "\nenc: " + String(enc_key) + "\nusername: " + String(username) + "\npassword: " + String(password) + "\nConnection: close");
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
  Serial.println(data);
  // Take only the content not the headers
  const char *PATTERN1 = "sub_hubs";
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
    }
  }

  // Format the data and ensure valid json
  data = String(target);
  Serial.println(String(target));
  free( target );
  return data;
}


// Access Point Connection Function that you can loop connect to Access Point until successful
String connectAP()
{
  /*String ip = "0.0.0.0";
  while (ip.equals("0.0.0.0"))
  {
    ip = wifi.connectAccessPoint(ssid, pass);
    if (!ip.equals("0.0.0.0"))
    {
      break;
    }
  }
  return ip;*/
}


/**
 * Encode data being sent over http connection
 *
 * @author Sam Mottley
 */
String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}
