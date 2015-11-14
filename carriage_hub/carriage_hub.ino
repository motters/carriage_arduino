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
//const uint8_t key[] = {'t','z','G','N','3','n','t','q','y','W','S','I','Z','M','L','6','n','Y','Y','H','e','B','2','E','p','D','k','2','I','H','9','2'};
const char username[] = "hub_225468";
const char enc_key[] = "tzGN3ntqyWSIZML6nYYHeB2EpDk2IH92";
char password[] = "password";
const char api_key[] = "8zA4N3vDrhgEFQugX4ThtO1Ch";


/**
 * Configure upload interval TEMP
 */
unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 100L * 1000L; // delay between updates, in milliseconds
boolean connected_to = false;


/**
 * Setup the application
 */
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  // Encrypt password
  //aes256_enc_single(key, password);
  //aes256_dec_single(key, password);
  
  // Start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP tring static IP instead.");
    // Try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
 
  httpRequest();
}


/**
 * Main program loop
 */
void loop() {
    
   if (client.available()) {
      char c = client.read();
      connected_to = true;
      Serial.print(c);
   }else{
      connected_to = false;
   }
   
   if(connected_to == false){
     if (millis() - lastConnectionTime > postingInterval) {
       httpRequest();
     }
   }
}


void httpRequest(){
 
  if (!client.connected()) {
    client.stop();
  }
 
 if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /api/v1/config/"+String(api_key)+" HTTP/1.1");
    client.println("Host: 192.168.33.17");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("api: "+String(api_key));
    client.println("enc: "+String(enc_key));
    client.println("username: "+String(username));
    client.println("Password: "+String(password));
    client.println("Connection: close");
    client.println();
    
    lastConnectionTime = millis();
  }else{
    Serial.println("connetion error");
  }
  
}


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
