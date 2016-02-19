// Include the reivent classes required
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// This will be set via the ardunio over serial
String connection_data_serial = "[{\"s\": \"MAINHUB_SSID\", \"p\":\"MAINHUB_PASSWORD\"}, {\"s\": \"SUBHUB-API-KEY\", \"p\": \"SUBHUB-PASSWORD\"}]"; // , {\"s\": \"CARRIAGEHUB_2\", \"p\": \"CARRIAGEHUB_PASSWORD\"}

// Store main hub connection details here once collected
String mainhub_ssid = "";
String mainhub_password = "";

// Setup some classes
ESP8266WebServer server(80);

void setup() {
  // Setup serial and allow device to settle
  delay(1000);
  Serial.begin(115200);
}

void loop() {
  // Wait for the ardunio to send the connection data
  bool wait = true;
  while (wait) {
    wait = false; // @todo this will be removed when arduino is connected
    if (Serial.available() > 0) {
      connection_data_serial = Serial.readString();
      wait = false;
    }
  }
  
  // De code system settings (dynamic buffer size still needs to be sorted)
  DynamicJsonBuffer jsonBuffer; // Use this for now but should be static to use less memory
  JsonArray& connection_data = jsonBuffer.parseArray(connection_data_serial);

  // Save the main hubs wireless connection details
  mainhub_ssid = connection_data[0]["s"].asString();
  mainhub_password = connection_data[0]["p"].asString();
  
  Serial.println(mainhub_ssid); Serial.println(mainhub_password);

  // Loop though all sub hubs
  int i;
  for (i = 0; i < (sizeof(connection_data) / sizeof(int)); i++) {
    // ignore zero
    if (i == 0)
      continue;

    bool connectedSub = false;
    WiFiClient client;
    while(!connectedSub){
      // Connect to them
      WiFi.begin(connection_data[1]["s"], connection_data[1]["p"]);
      Serial.println(connection_data[1]["s"].asString()); Serial.println(connection_data[1]["p"].asString());
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }
  
      // Send the main hubs wireless connection details
      delay(1000);
      if(client.connect("192.168.4.1", 80)){
          Serial.println("connected");
          connectedSub = true;
      }else{
         Serial.println("connected failed");
      }
    }
    
    // Compile header
    String url = "/main_hub?ssid=" + mainhub_ssid + "&pass=" + mainhub_password;
    Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + "192.168.4.1" + "\r\n" +
                 "Connection: close\r\n\r\n");

    // Wait a few
    delay(10);
    Serial.println("responce");
    // Read responce
    while(client.available()){
      String responce = client.readStringUntil('\r');
      Serial.println(responce);
    }
  }

  // Open the main hubs wireless access point
  char ssid_char[mainhub_ssid.length() + 1]; char pass_char[mainhub_password.length() + 1];
  mainhub_ssid.toCharArray(ssid_char, mainhub_ssid.length() + 1); mainhub_password.toCharArray(pass_char, mainhub_password.length() + 1);
  WiFi.softAP(ssid_char, pass_char, 1, 0); //CHANNEL = 1    HIDDEN = 1
  IPAddress myIP = WiFi.softAPIP();
  
  // Create a server on the main hub password page
  server.on("/data", handleConnection);
  server.begin();

  // Loop forever to manage connects
  while (true)
  {
    server.handleClient();
  }
}

void handleConnection()
{
  Serial.println("data recieved");

  // Send responce
  server.send(200, "text/html", "[{\"status\": \"200\"}] \r");
}
