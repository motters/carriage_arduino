// Include the reivent classes required
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// Set the default sub hub ssid and password (this will be printed on the side of the device and unique to each sub hub)
const char *ssid = "SUBHUB-API-KEY";
const char *password = "SUBHUB-PASSWORD";
bool isClosed = false;

// Store main hub connection details here once collected
String mainhub_ssid = "";
String mainhub_password = "";



// Setup some classes
ESP8266WebServer server(80);



/**
 * Setup the device and open web server for main hub to trasmit data to
 */
void setup() 
{
  // Setup serial and allow device to settle
  delay(1000);
  Serial.begin(115200);

  // Open a wifi connection
  WiFi.softAP(ssid, password, 1, 0); //CHANNEL = 1    HIDDEN = 1
  IPAddress myIP = WiFi.softAPIP();

  // Create a server on the main hub password page
  server.on("/main_hub", handleConnection);
  server.begin();
}



/**
 * Loop though the program functions
 */
void loop() 
{
  // If the main hub connection details are not collected then handle clients
  if(mainhub_ssid == ""){
    server.handleClient();
  }else if(isClosed == false){
    //If connection details are collected we'll shut down the wireless access point
    //WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    isClosed = true;
  }  
  
  if(mainhub_ssid != ""){
    // If sub hubs want to send data 
    if(Serial.available() > 0){
      // Read the data
      String data = Serial.readString();
      
      // Connect to the main hub
      char ssid_char[mainhub_ssid.length()+1]; char pass_char[mainhub_password.length()+1];
      mainhub_ssid.toCharArray(ssid_char, mainhub_ssid.length()+1); mainhub_password.toCharArray(pass_char, mainhub_password.length()+1);
      
      // Search for main hub and wait till it appears
      bool appeard = false;
      while(appeard == false){
         int n = WiFi.scanNetworks();
	 for (int i = 0; i < n; ++i) {
           String current_ssid = WiFi.SSID(i);
           Serial.println(current_ssid);
           if(current_ssid == mainhub_ssid)
             appeard = true;
         }
      }
      
      // Delay for a random time to reduce change of more than one sub hub connecting to main hub at same time
      //delay(random(10000, 990000));
      
      // Connect
      WiFi.begin(ssid_char, pass_char);
      WiFi.mode(WIFI_STA);
      // Wait for a successfull connection
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }

      // Create client instance
      WiFiClient client;
      
      // Send the data
      bool connectedMst = false;
      while(!connectedMst){
        Serial.println("Failed Connecting");
        if (client.connect("192.168.4.1", 80)) {
          connectedMst = true;
          //Logging of failed connection here
          Serial.println("Connected");
        }
      }
      
       // Create the request
      String url = "/data?data=" + data;
      Serial.println(data);
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + "192.168.4.1" + "\r\n" + 
                   "Connection: close\r\n\r\n");
      
      // Wait a few 
      delay(10);
      
      // Check responce
      while(client.available()){
        String responce = client.readStringUntil('\r');
        Serial.println(responce);
        if(responce == 0){
          // @todo log error
        }
      }
      
      client.stop();
          
    }
  }
}



/**
 * This function handles the setting of the main hubs ssid and password
 * The main hub will make a get request to http://192.168.4.1/main_hub?ssid={id}&pass={password}
 */
void handleConnection() 
{
  // Get password
  mainhub_ssid = server.arg("ssid");
  mainhub_password = server.arg("pass");

  // Send responce
  server.send(200, "text/html", "[{\"status\": \"200\"}]");
  
  // Wait for status to send
  delay(3000);
}
