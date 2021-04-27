/*
 Name:		Sketch1.ino
 Created:	12/20/2015 8:47:26 AM
 Author:	Nazman Gena
 */
#include "config.h"
#include "Relay.h"
#include "Channel.h"

const uint16_t wait_interval = 60000;

#ifdef CHILD_ID_RELAY1
Relay r1(RELAY1_PIN, CHILD_ID_RELAY1,RELAY1_NAME);
#endif
#ifdef CHILD_ID_RELAY2
Relay r2(RELAY2_PIN, CHILD_ID_RELAY2,RELAY2_NAME);
#endif
#ifdef CHILD_ID_RELAY3
Relay r3(RELAY3_PIN, CHILD_ID_RELAY3,RELAY3_NAME);
#endif
#ifdef CHILD_ID_RELAY4
Relay r4(RELAY4_PIN, CHILD_ID_RELAY4,RELAY4_NAME);
#endif

#ifdef CHILD_ID_CHANNEL1
Channel ch1(CHANNEL1_PIN, CHILD_ID_CHANNEL1, "");
#endif
#ifdef CHILD_ID_CHANNEL2
Channel ch2(CHANNEL2_PIN, CHILD_ID_CHANNEL2, "");
#endif
#ifdef CHILD_ID_CHANNEL3
Channel ch3(CHANNEL3_PIN, CHILD_ID_CHANNEL3, "");
#endif
#ifdef CHILD_ID_CHANNEL4
Channel ch4(CHANNEL4_PIN, CHILD_ID_CHANNEL4, "");
#endif

#ifdef TEMP_SENSOR
float prev_temp = 0, temp;
OneWire oneWire(CHILD_ID_TEMP);
DallasTemperature sensors(&oneWire);
// Arrays to hold device address
DeviceAddress insideThermometer;
// Gets temperature from DS sensors
float getTemp() {
	sensors.requestTemperatures();
	return DallasTemperature::toCelsius(sensors.getTempFByIndex(DS1820_INDEX));
}
// Sends temperature to gateway
void sendTemp() {
	MyMessage msg(CHILD_ID_TEMP, V_TEMP);
	temp = getTemp();
	if (temp != prev_temp) {
		send(msg.set(temp, 1));
	}
	prev_temp = temp;
}
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}
#endif

#ifdef PH_SENSOR
float getPH() {
	unsigned long int measure;  //Store the average value of the sensor feedback
	float b;
	int buf[10], temp;

	for (int i = 0; i<10; i++)       //Get 10 sample value from the sensor for smooth the value
	{
		buf[i] = analogRead(CHILD_ID_PH);
		wait(10);
	}
	for (int i = 0; i<9; i++)        //sort the analog from small to large
	{
		for (int j = i + 1; j<10; j++)
		{
			if (buf[i]>buf[j])
			{
				temp = buf[i];
				buf[i] = buf[j];
				buf[j] = temp;
			}
		}
	}
	measure = 0;
	for (int i = 2; i<8; i++)                      //take the average value of 6 center sample
		measure += buf[i];

	//int measure = analogRead(CHILD_ID_PH);
	Serial.print("Measured value: ");
	Serial.print(measure);
	float pHVol = static_cast<float>(measure * 5.0 / 1024 / 6);
	Serial.print("\nVoltage: ");
	Serial.print(pHVol, 3);
	// PH_step = (voltage@PH7 - voltage@PH4) / (PH7 - PH4)
	// PH_probe = PH7 - ((voltage@PH7 - voltage@probe) / PH_step)
	return 6.88 + ((3.228 - pHVol) / 0.17429);
}

void sendPH() {
	MyMessage msg(CHILD_ID_PH, V_PH);
	send(msg.set(getPH(), 1));
}
#endif

void before()
{
	
}

// the setup function runs once when you press reset or power the board
void setup() {
#ifndef MY_DEBUG
	Serial.begin(115200);
#endif 
#ifdef TEMP_SENSOR
	sensors.begin();
	Serial.print("Locating devices...");
	Serial.print(sensors.getDeviceCount(), DEC);
	Serial.println(" devices.");
	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (sensors.isParasitePowerMode()) Serial.println("ON");
	else Serial.println("OFF");
	if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
	Serial.print("Device 0 Address: ");
	printAddress(insideThermometer);
	Serial.println();
	sensors.setWaitForConversion(false);
#endif
#ifdef CHILD_ID_RELAY1
	r1.begin();
#endif	
#ifdef CHILD_ID_RELAY2	
	r2.begin();
#endif	
#ifdef CHILD_ID_RELAY3	
	r3.begin();
#endif	
#ifdef CHILD_ID_RELAY4	
	r4.begin();
#endif

#ifdef CHILD_ID_CHANNEL1
	ch1.begin();
#endif	
#ifdef CHILD_ID_CHANNEL2	
	ch2.begin();
#endif	
#ifdef CHILD_ID_CHANNEL3	
	ch3.begin();
#endif	
#ifdef CHILD_ID_CHANNEL4	
	ch4.begin();
#endif	
}

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo(SN, SV);

	// Register all sensors to gw (they will be created as child devices)
#ifdef CHILD_ID_RELAY1	
	present(CHILD_ID_RELAY1, S_BINARY, r1.getDescr());
#endif	
#ifdef CHILD_ID_RELAY2	
	present(CHILD_ID_RELAY2, S_BINARY, r2.getDescr());
#endif	
#ifdef CHILD_ID_RELAY3	
	present(CHILD_ID_RELAY3, S_BINARY, r3.getDescr());
#endif	
#ifdef CHILD_ID_RELAY4	
	present(CHILD_ID_RELAY4, S_BINARY, r4.getDescr());
#endif	

#ifdef CHILD_ID_CHANNEL1
	present(CHILD_ID_CHANNEL1, S_DIMMER);
#endif		
#ifdef CHILD_ID_CHANNEL2	
	present(CHILD_ID_CHANNEL2, S_DIMMER);
#endif		
#ifdef CHILD_ID_CHANNEL3	
	present(CHILD_ID_CHANNEL3, S_DIMMER);
#endif		
#ifdef CHILD_ID_CHANNEL4	
	present(CHILD_ID_CHANNEL4, S_DIMMER);
#endif	

#ifdef TEMP_SENSOR	
	present(CHILD_ID_TEMP, S_WATER_QUALITY);
#endif
#ifdef PH_SENSOR
	present(CHILD_ID_PH, S_WATER_QUALITY);
#endif
}

void loop() {

#ifdef TEMP_SENSOR	
	sendTemp();
#endif
#ifdef PH_SENSOR
	sendPH();
#endif
#ifdef CHILD_ID_RELAY1
	r1.sendState();
#endif
#ifdef CHILD_ID_RELAY2
	r2.sendState();
#endif
#ifdef CHILD_ID_RELAY3
	r3.sendState();
#endif
#ifdef CHILD_ID_RELAY4
	r4.sendState();
#endif
	wait(wait_interval);
	
	/*ch1.sendState();
	ch2.sendState();
	ch3.sendState();
	ch4.sendState();*/

	sendHeartbeat();
}

void receive(const MyMessage &message) {
	bool prevState, newState;
	float newValue;

	if (message.isEcho()) {
		Serial.println("This is an Echo from gateway");
		return;
	}

		if (message.type == V_STATUS) {

			Serial.print("\nIncoming change for sensor: ");
			Serial.print(message.sensor);
			Serial.print(", New status: ");
			Serial.println(message.getBool());

			switch (message.sensor) {
#ifdef CHILD_ID_RELAY1
			case CHILD_ID_RELAY1: {
				newState = message.getBool();
				newState ? r1.on() : r1.off();
				break;
			}
#endif			
#ifdef CHILD_ID_RELAY2			
			case CHILD_ID_RELAY2: {
				newState = message.getBool();
				newState ? r2.on() : r2.off();
				break;
			}
#endif			
#ifdef CHILD_ID_RELAY3			
			case CHILD_ID_RELAY3: {
				newState = message.getBool();
				newState ? r3.on() : r3.off();
				break;
			}
#endif			
#ifdef CHILD_ID_RELAY4			
			case CHILD_ID_RELAY4: {
				newState = message.getBool();
				newState ? r4.on() : r4.off();
				break;
			}
#endif	
#ifdef CHILD_ID_CHANNEL1		
			case CHILD_ID_CHANNEL1: {
				newState = message.getBool();
				newState ? ch1.on() : ch1.off();
				break;
			}
#endif			
#ifdef CHILD_ID_CHANNEL2			
			case CHILD_ID_CHANNEL2: {
				newState = message.getBool();
				newState ? ch2.on() : ch2.off();
				break;
			}
#endif			
#ifdef CHILD_ID_CHANNEL3			
			case CHILD_ID_CHANNEL3: {
				newState = message.getBool();
				newState ? ch3.on() : ch3.off();
				break;
			}
#endif			
#ifdef CHILD_ID_CHANNEL4			
			case CHILD_ID_CHANNEL4: {
				newState = message.getBool();
				newState ? ch4.on() : ch4.off();
				break;
			}
#endif
		}
	}
		if (message.type == V_PERCENTAGE) {
			Serial.print("\nIncoming change for channel: ");
			Serial.print(message.sensor);
			Serial.print(", New value: ");
			newValue = (message.getFloat() / 100) * 255;
			Serial.println(message.getUInt());

			switch (message.sensor) {
#ifdef CHILD_ID_CHANNEL1					
			case CHILD_ID_CHANNEL1: {
				ch1.pwm(newValue);
				break;
			}
#endif			
#ifdef CHILD_ID_CHANNEL2			
			case CHILD_ID_CHANNEL2: {
				ch2.pwm(newValue);
				break;
			}
#endif			
#ifdef CHILD_ID_CHANNEL3			
			case CHILD_ID_CHANNEL3: {
				ch3.pwm(newValue);
				break;
			}
#endif			
#ifdef CHILD_ID_CHANNEL4			
			case CHILD_ID_CHANNEL4: {
				ch4.pwm(newValue);
				break;
			}
#endif
			}
		}
	
}
