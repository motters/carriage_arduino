/**
 * Include libs
 */
#include <ArduinoJson.h>


/**
 * Temp data which represents data retrieved via the wireless link
 *
 *  @author Sam Mottley
 */
String modules_config = "[{\"module_connections\":\"SHA-A3\",\"module\":\"2\",\"interval\":\"5000\"}, {\"module_connections\":\"SHA-A4\",\"module\":\"3\",\"interval\":\"1000\"}]";


/**
 * Declar global varables required
 *
 *  @author Sam Mottley
 */
unsigned long module_times[255]; // This is to be changed for C++ map function or vector to enable auto sizing 
//String module_buffer[10][50]; // Thus is to be changed for C++ map fucntion or vector to enable auto sizing 

/**
 * Setup the application
 *
 *  @author Sam Mottley
 */
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
}


/**
 * Main program loop
 *
 * @author Sam Mottley
 */
void loop() {
  
  // De code system settings (dynamic buffer size still needs to be sorted)
  DynamicJsonBuffer jsonBuffer; // Use this for now but should be static to use less memory
  JsonArray& modules_object = jsonBuffer.parseArray(modules_config);
  
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
        interval = modules_object[i]["interval"];
        currentMillis = millis();
        
        // Check last recorded time    int atoi(interval)
        if (currentMillis - module_times[i] >= atoi(interval)) 
        {
            // Save the last time you blinked the LED
            module_times[i] = currentMillis;
            
            // Decide which function to run
            module_type = modules_object[i]["module"];
            String connection_buf = modules_object[i]["module_connections"];
            
            switch(atoi(module_type))
            {
                case 2:
                  readTemperture(connection_buf);
                break;
                case 3:
                  readVibration(connection_buf);
                break;
                case 4:
                  readSoundLevel(connection_buf);
                break;
                case 5:
                  readVoltages(connection_buf);
                break;
            }
        }
     }
     
     
     // Check buffer size
       // Larger than X
          // Send buffer via wireless link to sub hub 
          // Clear buffer
  }
}


/**
 * This function reads the temperture relative to the connections
 */
void readTemperture(String connection)
{
  Serial.print("Read Temperture Value: ");
  Serial.println(connection);
}



/**
 * This function reads the vibration relative to the connections
 */
void readVibration(String connection)
{
  Serial.print("Read Vibration Value: ");
  Serial.println(connection);
}



/**
 * This function reads the sound levels relative to the connections
 */
void readSoundLevel(String connection)
{
  Serial.print("Read Sound Level: ");
  Serial.println(connection);
}



/**
 * This function reads voltages relative to the connections
 */
void readVoltages(String connection)
{
  Serial.print("Read Voltage Value: "); 
  Serial.println(connection);
}



