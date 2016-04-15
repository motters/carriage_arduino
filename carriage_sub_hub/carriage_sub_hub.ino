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
// [{"mc":"SH-D-3","t":"2","in":"4000"}, {"mc":"SH-I2C-105","t":"3","in":"2000"}, {"mc":"SH-SER-10-11","t":"5","in":"10000"}]     [{\"mc\":\"SH-D-3\",\"t\":\"2\",\"in\":\"4000\"}, {\"mc\":\"SH-I2C-105\",\"t\":\"3\",\"in\":\"2000\"}]
String modules_config = "";


/**
 * Declar / define some function
 */
void readTemperture(String connection);
void readVibration(String connection);
void readAirFlow(String connection);
void readGPS(String connection);
void readGPS(String connection);
boolean sendBuffer(String connection);
int count_commas(String s);
String timestamp();
String split(String in_str, int id);

/**
 * Declar global varables required
 *
 *  @author Sam Mottley
 */
int maxNumberValues = 1;
//unsigned long module_times[20]; // This is to be changed for C++ map function or vector to enable auto sizing 
std::map< int, unsigned long > module_times; 
std::map< String, String > buffer_container; 


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
	//String modules_config_setup = "";
	while(modules_config == ""){
		if(Serial.available() > 0){
			// Set the string
			modules_config = Serial.readString();
			//Serial.println(modules_config);
			//setup = true;
		}
	}

	// Init libs
	Wire.begin();
	RTC.begin();
  //RTC.begin(DateTime(F(__DATE__), F(__TIME__)));  
	delay(500);

  // Following line sets the RTC to the date & time this sketch was compiled
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

	// Check real time clock
	if (! RTC.isrunning()) {
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //DateTime now = RTC.now();
    //Serial.println(String(now.unixtime()));
		//Serial.println("Couldn't find RTC");
		//while (1);
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
  //modules_config = "";
  
	// Delcar Vars
	int i = 0;
	const char* interval;
	unsigned long currentMillis;
	const char* module_type;
	String connection;

	// Config loaded now loop
	while(true)
	{
		i = 0;
		// Loop through all modules in configuration
		for (i = 0; i < (sizeof(modules_object)/sizeof(int)); i++) 
		{
    			// Does module need recording
    			interval = modules_object[i]["in"];
    			currentMillis = millis();
    
    			// Check last recorded time    int atoi(interval)
    			if (currentMillis - module_times[i] >= atoi(interval)) 
    			{
        				// Save the last time you blinked the LED
        				module_times[i] = currentMillis;
        
        				// Decide which function to run
        				module_type = modules_object[i]["t"].asString(); // .asString()
        				String connection_buf = modules_object[i]["mc"];
        
        				switch(atoi(module_type))
        				{
            				case 2:
            					readTemperture(connection_buf);
            					break;
            				case 3:
            					readVibration(connection_buf);
            					break;
            				case 4:
            					readAirFlow(connection_buf);
            					break;
                    case 5:
                      readGPS(connection_buf);
                      break;
                }
    			  }


      			// Check size of buffer
      			String connection_buf = modules_object[i]["mc"];        
      			if(count_commas(buffer_container[connection_buf]) >= maxNumberValues){
      				// Send buffer via wireless link to sub hub 
      				sendBuffer(connection_buf);
      			}  
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
	DateTime now = RTC.now();
	return String(now.unixtime());
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
 */
boolean sendBuffer(String connection)
{ 
	// Find module type
	DynamicJsonBuffer jsonBuffer; // Use this for now but should be static to use less memory
	JsonArray& modules_object = jsonBuffer.parseArray(modules_config);

	// Loop through all modules in configuration
	String type = "0"; int i = 0;
	for (i = 0; i < (sizeof(modules_object)/sizeof(int)); i++) 
	{
		if(modules_object[i]["mc"] == connection){
			type = modules_object[i]["t"].asString();
			break;
		}
	}

	// Send the current buffer
	Serial.print(connection+"!"+type+"!"+ buffer_container[connection]+"}");

	// Clear the current buffer
	buffer_container[connection] = "";

	// Return true
	return true;
}


/**
 * This function reads the temperture relative to the connections
 */
void readTemperture(String connection)
{
	String temp = split(connection, 2);
	uint8_t port = temp.toInt();
	//Serial.println(port);
	// Init temperature reader class 
	DHT dht(port, 11);
	dht.begin();

	//Read date from sensor
	float t = dht.readTemperature();
	float h = dht.readHumidity();

	// Add recorded value to buffer
	buffer_container[connection] += "," + timestamp() + "@" + String(t) + ":" + String(h); 
}


//0x50 0x68
/**
 * This function reads the vibration relative to the connections
 */
void readVibration(String connection)
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

	buffer_container[connection] += ","  + timestamp() + "@" + String(AcX)+":"+String(AcY)+":"+String(AcZ);//+":"+String(Tmp)+":"+String(GyX)+":"+String(GyY)+":"+String(GyZ);
}


/**
 * This function reads the sound levels relative to the connections SH-12BIT-0
 */
void readAirFlow(String connection)
{
  // Get the channel to be read
  String channel_string = split(connection, 2);
  int channel = channel_string.toInt(); 
  
  // Set default value for ADC reading
  int adcvalue = 0;

  // Set command bits for ADC - start, mode, chn (3), dont care (3)
  ::byte commandbits = B11000000; 
  //unsigned char commandbits = B11000000; 

  // Select channel to read
  commandbits|=channel<<3; //commandbits|=((channel-1)<<3); We'll start from zero for now

  // Select adc
  ::digitalWrite(10, LOW); 

  // Ready bits to be wrote
  for (int i=7; i>=3; i--){
    ::digitalWrite(11, commandbits&1<<i);
    //cycle clock
    ::digitalWrite(12, HIGH);
    ::digitalWrite(12, LOW);    
  }

  // Ignores 2 null bits
  ::digitalWrite(12, HIGH);    
  ::digitalWrite(12, LOW);
  ::digitalWrite(12, HIGH);  
  ::digitalWrite(12, LOW);

  // Read bits from ADC
  for (int i=11; i>=0; i--)
  {
    adcvalue+=::digitalRead(13)<<i;
    //cycle clock
    ::digitalWrite(12, HIGH);
    ::digitalWrite(12, LOW);
  }

  // Turn off device ADC
  ::digitalWrite(10, HIGH); 

  // Save to buffer
  buffer_container[connection] += ","  + timestamp() + "@" + String(adcvalue);
}


/**
 * This function reads the sound levels relative to the connections SH-SER-10-11
 */
void readGPS(String connection)
{
  //Serial.print("in function");
  // Get the connection data
  String rx = split(connection, 2);
  String tx = split(connection, 3);
  //Serial.println(rx);Serial.println(tx);

  // Start software serial
  SoftwareSerial gps_serial(rx.toInt(), tx.toInt()); // RX, TX
  gps_serial.begin(9600);
  delay(200);
  //Serial.println("setup software serial");

  unsigned long start = millis();
  
  // Wait for next serial dump
  while(gps_serial.available()){  
        //Serial.print(".");
        // Read the data
        char c = gps_serial.read();
        //Serial.println(c);
        // Encode data into global gps buffer
        if (gps.encode(c)) {
              // Add encoded data to global buffer
              //Serial.println(gps.satellites.value());
              buffer_container[connection] += ","  + timestamp() + "@" + String(gps.satellites.value()) + ":" + String(gps.location.lat(), 6) + ":" + String(gps.location.lng(), 6);
              break;
        }
        // Wait too long we'll break out
        while (millis() - start < 5000) {
            break;
        }
  }
  
  // Close serial
  gps_serial.end();
}

