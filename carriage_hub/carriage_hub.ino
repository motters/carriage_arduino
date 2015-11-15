/**
 * Include libs
 */
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <AESLib.h>


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


/**
 * Configure upload interval TEMP
 *
 * @author Sam Mottley
 */
unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 100L * 200L; // delay between updates, in milliseconds
boolean connected_to = false;
String data = "";


/**
 * Setup the application
 *
 * @author Sam Mottley
 */
void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  // Encrypt
  //aes256_enc_single(key, password);
  //aes256_dec_single(key, password);
  
  // Start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP tring static IP instead.");
    // Try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
 
  // Get the hubs and associated sub hubs configuration
  httpRequestConfig();
}


/**
 * Main program loop
 *
 * @author Sam Mottley
 */
void loop() 
{
   
   if (millis() - lastConnectionTime > postingInterval) {
     lastConnectionTime = millis();
     if (!client.connected()) {
       httpRequestConfig();
     }
   }
   
}


/**
 * Retreive the hubs and sub hub configuration from server
 *
 * @author Sam Mottley
 */
void httpRequestConfig()
{
 // Ensure socket is not in use
 if (!client.connected()) {
    client.stop();
 }
 
 // When socket free open a connection
 if (client.connect(server, 80)) {
    // Make the http request to server
    client.println("GET /api/v1/config/"+String(api_key)+" HTTP/1.1\nHost: 192.168.33.17\napi: "+String(api_key)+"\nenc: "+String(enc_key)+"\nusername: "+String(username)+"\npassword: "+String(password)+"\nConnection: close");
    client.println();
 }
 
 // Wait for server to recieve request 
 while(client.connected() && !client.available()) delay(1);
  
 // Retrieve data from server and store in buffer
 while (client.connected() || client.available()) {
      char c = client.read();
      if (c == '\n') { data += " "; } else { data += c; }
 }
 
 // Close the connection / socket 
 client.stop();
  
 // Print the buffered data
 Serial.println(data);
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

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
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
