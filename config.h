// Node ID
#define MY_NODE_ID 51

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

// Enable debug prints to serial monitor
#define MY_DEBUG 

//#define MY_DEBUG_VERBOSE_RF24

// Enable and select radio type attached
#define MY_RADIO_RF24

#define MY_SIGNAL_REPORT_ENABLED

#define MY_RF24_PA_LEVEL RF24_PA_MAX


// For Mega2560
#define MY_RF24_CE_PIN 49
#define MY_RF24_CS_PIN 53

#define MY_SOFTSPI
#define MY_SOFT_SPI_SCK_PIN 52
#define MY_SOFT_SPI_MISO_PIN 50
#define MY_SOFT_SPI_MOSI_PIN 51

#include <MySensors.h>
#include <MyConfig.h>




#define SN "Aqarium light"
#define SV "1.0.2"

#define INVERSE_LOGIC

//#define NUM_OF_RELAYS 2


// Relay defenitions
//#define CHILD_ID_RELAY1 1   
//#define CHILD_ID_RELAY2 2   
#define CHILD_ID_RELAY3 3	
#define CHILD_ID_RELAY4 4

#ifdef CHILD_ID_RELAY1
#define RELAY1_PIN 37  
#define RELAY1_NAME "CO2"
#endif
#ifdef CHILD_ID_RELAY2
#define RELAY2_PIN 36  
#define RELAY2_NAME "Heater"
#endif
#ifdef CHILD_ID_RELAY3
#define RELAY3_PIN 35
#define RELAY3_NAME "Light"
#endif
#ifdef CHILD_ID_RELAY4
#define RELAY4_PIN 34
#define RELAY4_NAME "Filter"  
#endif
// Lighting channel defenitions
//#define CHILD_ID_CHANNEL1 10
//#define CHILD_ID_CHANNEL2 11
//#define CHILD_ID_CHANNEL3 12
//#define CHILD_ID_CHANNEL4 13

#ifdef CHILD_ID_CHANNEL1
#define CHANNEL1_PIN 10
#define CHANNEL1_NAME "Red"
#endif
#ifdef CHILD_ID_CHANNEL2
#define CHANNEL2_PIN 11
#define CHANNEL2_NAME "Blue"
#endif
#ifdef CHILD_ID_CHANNEL3
#define CHANNEL3_PIN 12
#define CHANNEL3_NAME "Warm White"
#endif
#ifdef CHILD_ID_CHANNEL4
#define CHANNEL4_PIN 13
#define CHANNEL4_NAME "Cold White"
#endif


// INVERSE_LOGIC
#ifdef INVERSE_LOGIC
#define RELAY_ON 0
#define RELAY_OFF 1
#else
#define RELAY_ON 1
#define RELAY_OFF 0
#endif 

// Temperature sensor
//#define TEMP_SENSOR

// pH sensor
//#define PH_SENSOR

#ifdef TEMP_SENSOR
#include <Wire.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#endif

// Temperature sensor
#ifdef TEMP_SENSOR
#define CHILD_ID_TEMP 32
#define DS1820_INDEX 0
#endif

// pH sensor
#ifdef PH_SENSOR
#define CHILD_ID_PH A0
#endif
