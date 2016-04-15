/**
* Include libs
*/
#include <ArduinoJson.h>
#include <StandardCplusplus.h>
#include <map>
#include <Wire.h>
#include "DHT.h"
#include "RTClib.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


/**
 * Temp data which represents data retrieved via the wireless link
 *
 *  @author Sam Mottley
 */
// [{"mc":"SH-D-3","t":"2","in":"4000"}, {"mc":"SH-I2C-105","t":"3","in":"2000"}, {"mc":"SH-SER-5-6","t":"5","in":"10000"}, {"mc":"SH-12BIT-0","t":"4","in":"5000"}]     [{\"mc\":\"SH-D-3\",\"t\":\"2\",\"in\":\"4000\"}, {\"mc\":\"SH-I2C-105\",\"t\":\"3\",\"in\":\"2000\"}]
String modules_config = "";


/**
 * Declar / define some function
 */
void readTemperture(String connection, String type);
void readVibration(String connection, String type);
void readAirFlow(String connection, String type);
void readGPS(String connection, String type);
void readGPS(String connection, String type);
boolean sendBuffer(String connection, String type);
int count_commas(String s);
String timestamp();
String split(String in_str, int id);

/**
 * Declar global varables required
 *
 *  @author Sam Mottley
 */
//unsigned long module_times[20]; // This is to be changed for C++ map function or vector to enable auto sizing 
std::map< int, unsigned long > module_times; 
std::map< String, String > buffer_container; //This is currently not used due to no buffer being used as memeory is so small
int maxNumberValues = 1; // This is currently not used due to no buffer being used as memeory is so small

/**
 * Declar global varables for module methods
 *
 *  @author Sam Mottley & Mark 
 */
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // For accler
RTC_DS1307 RTC; // For real time
TinyGPSPlus gps;

/**
 * Setup the application
 *
 *  @author Sam Mottley
 */
void setup() 
{
	// Open serial communications and wait for port to open:
	Serial.begin(115200);
  
	// Set the config string
	while(modules_config == ""){
		if(Serial.available() > 0){
			// Set the string
			modules_config = Serial.readString();
		}
	}

	// Init libs
	Wire.begin();
	RTC.begin();
	delay(500);

  // Check real time clock
	if (! RTC.isrunning()) {
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
}


/**
 * Main program loop
 *
 * @author Sam Mottley
 */
void loop() 
{ 
	// De code system settings (dynamic buffer size still needs to be sorted)
	DynamicJsonBuffer jsonBuffer; // Use this for now but should be static to use less memory
	JsonArray& modules_object = jsonBuffer.parseArray(modules_config);

  // Free some SRAM
  modules_config = "";
  
	// Delcar Vars
	int i = 0;
	const char* interval;
	unsigned long currentMillis;
	const char* module_type;
	String connection;
  String connection_buf;

	// Config loaded now loop
	while(true)
	{
		i = 0;
		// Loop through all modules in configuration
		for (i = 0; i <= (sizeof(modules_object)/sizeof(int)); i++) 
		{
          // Config some timing
          interval = modules_object[i]["in"]; 
    			currentMillis = millis();
    
    			// Check last recorded time
    			if (currentMillis - module_times[i] >= atoi(interval)) 
    			{
        				// Save the last time you blinked the LED
        				module_times[i] = currentMillis;
        
        				// Decide which function to run
        				module_type = modules_object[i]["t"].asString(); // .asString()
               
        				connection_buf = modules_object[i]["mc"].asString();
                
        				switch(atoi(module_type))
        				{
            				case 2:
            					readTemperture(connection_buf, module_type);
            					break;
            				case 3:
            					readVibration(connection_buf, module_type);
            					break;
            				case 4:
            					readAirFlow(connection_buf, module_type);
            					break;
                    case 5:
                      readGPS(connection_buf, module_type);
                      break;
                }
    			  }


      			// Check size of buffer (This is currently not used due to no buffer being used as memeory is so small)
      			/*String connection_buf = modules_object[i]["mc"];        
      			if(count_commas(buffer_container[connection_buf]) >= maxNumberValues){
      				// Send buffer via wireless link to sub hub 
      				sendBuffer(connection_buf, modules_object[i]["t"].asString());
      			}*/  
		   }
	  }
}


/**
 * Count the number of commas in a string
 */
int count_commas(String s) {
	int count = 0;

	for (int i = 0; i < s.length(); i++)
		if (s[i] == ',') count++;

	return count;
}


/** 
 * Read the current time stamp(Unix)
 */
String timestamp()
{
  while(true){
       // Get time
       DateTime now = RTC.now();

       // Check time
       if(String(now.unixtime()) > "1460762203")
            return String(now.unixtime());
  }
}


/** 
 * Seperates connection data
 */
String split(String in_str, int id) {

	char sz[in_str.length()+1];
	in_str.toCharArray(sz, in_str.length()+1); 

	char *p = sz;
	char *str;
	int id_c = 0;
	while ((str = strtok_r(p, "-", &p)) != NULL){
		if(id_c == id){
			return String(str);
		}else{
			id_c++;
		}
	}
	return "0";
}

/** 
 * Send data via the wireless link and clear the data buffer
 * This is currently not used due to no buffer being used as memeory is so small
 */
boolean sendBuffer(String connection, String type)
{ 
	// Send the current buffer
	Serial.print(connection+"!"+type+"!"+ buffer_container[connection]+"}");

	// Clear the current buffer
	buffer_container[connection] = "";

	// Return true
	return true;
}


/**
 * This function reads the temperture relative to the connections
 * Example Config Data: [{"mc":"SH-D-3","t":"2","in":"4000"}]
 */
void readTemperture(String connection, String type)
{
  // Get the port information
	String temp = split(connection, 2);
	uint8_t port = temp.toInt();

	// Init temperature reader class 
	DHT dht(port, 11);
	dht.begin();

	//Read date from sensor
	float t = dht.readTemperature();
	float h = dht.readHumidity();

	// Add recorded value to buffer
	//buffer_container[connection] += "," + timestamp() + "@" + String(t) + ":" + String(h); 
  Serial.print(connection+"!"+type+"!" + timestamp() + "@" + String(t) + ":" + String(h)+"}");
}


/**
 * This function reads the vibration relative to the connections
 * Example Config Data: [{"mc":"SH-I2C-105","t":"3","in":"2000"}]
 */
void readVibration(String connection, String type)
{
	String temp = split(connection, 2);
	int port = temp.toInt();
	//Serial.println(port);
	Wire.beginTransmission(port);
	Wire.write(0x6B);  // PWR_MGMT_1 register
	Wire.write(0);     // set to zero (wakes up the MPU-6050)
	Wire.endTransmission(true);

	Wire.beginTransmission(port);
	Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
	Wire.endTransmission(false);
	Wire.requestFrom(port, 14, true); // request a total of 14 registers

	//Reads 2 bits for each variable
	//These are the registers that can be found on the datasheet
	AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
	AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
	GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
	GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
	GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

	//buffer_container[connection] += ","  + timestamp() + "@" + String(AcX)+":"+String(AcY)+":"+String(AcZ);//+":"+String(Tmp)+":"+String(GyX)+":"+String(GyY)+":"+String(GyZ);
  Serial.print(connection+"!"+type+"!" + timestamp() + "@" + String(AcX)+":"+String(AcY)+":"+String(AcZ)+"}");
}


/**
 * This function reads the sound levels relative to the connections 
 * Example Config Data: [{"mc":"SH-12BIT-0","t":"4","in":"5000"}] 
 */
void readAirFlow(String connection, String type)
{
  //set pin modes 
  pinMode(10, OUTPUT); 
  pinMode(11, OUTPUT); 
  pinMode(12, INPUT); 
  pinMode(13, OUTPUT); 
  //disable device to start with 
  digitalWrite(10,HIGH); 
  digitalWrite(11,LOW); 
  digitalWrite(13,LOW); 
 
  // Get the channel to be read
  String channel_string = split(connection, 2);
  int channel = channel_string.toInt(); 
  
  // Set default value for ADC reading
  int adcvalue = 0;

  // Set command bits for ADC - start, mode, chn (3), dont care (3)
  byte commandbits = B11000000; 
  //unsigned char commandbits = B11000000; 

  // Select channel to read
  commandbits|=channel<<3; //commandbits|=((channel-1)<<3); We'll start from zero for now

  // Select adc
  digitalWrite(10, LOW); 

  // Ready bits to be wrote
  for (int i=7; i>=3; i--){
    digitalWrite(11, commandbits&1<<i);
    //cycle clock
    digitalWrite(13, HIGH);
    digitalWrite(13, LOW);    
  }

  // Ignores 2 null bits
  digitalWrite(13, HIGH);    
  digitalWrite(13, LOW);
  digitalWrite(13, HIGH);  
  digitalWrite(13, LOW);

  // Read bits from ADC
  for (int i=11; i>=0; i--)
  {
    adcvalue+=digitalRead(12)<<i;
    //cycle clock
    digitalWrite(13, HIGH);
    digitalWrite(13, LOW);
  }

  // Turn off device ADC
  digitalWrite(10, HIGH); 

  // Save to buffer
  //buffer_container[connection] += ","  + timestamp() + "@" + String(adcvalue);
  Serial.print(connection+"!"+type+"!" + timestamp() + "@" + String(adcvalue) +"}");
}

/**
 * This function reads the sound levels relative to the connections    
 * Example config data: [{"mc":"SH-SER-5-6","t":"5","in":"10000"}] 
 */
void readGPS(String connection, String type)
{
  // Get the connection data
  String rx = split(connection, 2);
  String tx = split(connection, 3);

  // Start software serial
  SoftwareSerial gps_serial(rx.toInt(), tx.toInt()); // RX, TX
  gps_serial.begin(9600); 

  // We will spend max of 5 seconds trying to read the GPS position
  unsigned long start = millis();
  while (millis() - start < 5000) {
        if (gps_serial.available()) {
              char c = gps_serial.read();
              if (gps.encode(c)) {
                    // Add encoded data to global buffer
                    //buffer_container[connection] += ","  + timestamp() + "@" + String(gps.satellites.value()) + ":" + String(gps.location.lat(), 6) + ":" + String(gps.location.lng(), 6);
                    Serial.print(connection+"!"+type+"!" + timestamp() + "@" + String(gps.satellites.value()) + ":" + String(gps.location.lat(), 6) + ":" + String(gps.location.lng(), 6) +"}");
                    break;
              }
        }
  }
  
  // Close serial
  gps_serial.end();
}

