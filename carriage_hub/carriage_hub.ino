/**
 * Include libs
 */
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <AESLib.h>


/**
 * Setup ethernet connection
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
 */
const uint8_t key[] = {'t','z','G','N','3','n','t','q','y','W','S','I','Z','M','L','6','n','Y','Y','H','e','B','2','E','p','D','k','2','I','H','9','2'};
const char username[] = "hub_225468";
char password[] = "password";
const char api_key[] = "8zA4N3vDrhgEFQugX4ThtO1Ch";


/**
 * Setup the application
 */
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // Start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP tring static IP instead.");
    // Try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  
   // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET / HTTP/1.1");
    client.println("Host: 192.168.33.17");
    client.println("Connection: close");
    client.println();
  }
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}


/**
 * Main program loop
 */
void loop() {
  // put your main code here, to run repeatedly:
  aes256_enc_single(key, password);
  aes256_dec_single(key, password);

  
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}
