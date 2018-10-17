/*
 Name:		Sketch1.ino
 Created:	12/20/2015 8:47:26 AM
 Author:	admin
 */
// Node ID
#define MY_NODE_ID 54

// Enable debug prints to serial monitor
#define MY_DEBUG 

//#define MY_DEBUG_VERBOSE_RF24

// Enable and select radio type attached
#define MY_RADIO_NRF24
 //#define MY_RADIO_RFM69


//#define MY_SIGNAL_REPORT_ENABLED

#define MY_RF24_PA_LEVEL RF24_PA_MAX
//#define MY_RF24_PA_LEVEL RF24_PA_MIN

// For Mega2560
#define MY_RF24_CE_PIN 49
#define MY_RF24_CS_PIN 53

#define MY_SOFTSPI
#define MY_SOFT_SPI_SCK_PIN 52
#define MY_SOFT_SPI_MISO_PIN 50
#define MY_SOFT_SPI_MOSI_PIN 51


#define INVERSE_LOGIC

 // Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MySensors.h>
#include <MyConfig.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <OneWire.h>


// Digital pins
#define RELAY1_PIN 37  // Light
#define RELAY2_PIN 36  // CO2
#define RELAY3_PIN 35 // Filter
#define RELAY4_PIN 34 // Heater


#define CHILD_ID_RELAY1 1   
#define CHILD_ID_RELAY2 2   
#define CHILD_ID_RELAY3 3	
#define CHILD_ID_RELAY4 4

#ifdef INVERSE_LOGIC
#define RELAY_ON 0
#define RELAY_OFF 1

#else
#define RELAY_ON 1
#define RELAY_OFF 0

#endif // INVERSE_LOGIC

// Channel pins
#define CHANNEL1_PIN 10
#define CHANNEL2_PIN 11
#define CHANNEL3_PIN 12
#define CHANNEL4_PIN 13

#define CHILD_ID_CHANNEL1 10
#define CHILD_ID_CHANNEL2 11
#define CHILD_ID_CHANNEL3 12
#define CHILD_ID_CHANNEL4 13

#define LIGHT_OFF 0
#define LIGHT_ON 1

// Temperature sensor
#define CHILD_ID_TEMP 32
#define DS1820_INDEX 0

// pH sensor
#define CHILD_ID_PH A0

long cur_time = 0;

class Relay {
public:
	Relay(uint8_t pin, uint8_t address) : _pin(pin), _address(address) { 
		_state = 0; 
		_msg = new MyMessage(_address, V_STATUS);
	};
	void begin() {
		pinMode(_pin, OUTPUT);
		digitalWrite(_pin, RELAY_OFF);
		_state = loadState(_address);
		Serial.print("Relay: ");
		Serial.print(_address);
		Serial.print(" .Loading previous state: ");
		Serial.println(_state);
#ifdef INVERSE_LOGIC
		//digitalWrite(_pin, _state ? RELAY_OFF : RELAY_ON);
		_state ? off() : on();
#else
		_state ? on() : off();
		//digitalWrite(_pin, _state ? RELAY_ON : RELAY_OFF);
#endif
				
	};

	bool proceed(long start_time, long duration) {
		
		if (cur_time == 0) { return 0; }
		if ((cur_time >= start_time) && cur_time < (start_time + duration) && _state == RELAY_OFF) {
			Serial.print("Powering on by timer relay: ");
			Serial.print(_address);			
			this->on();			
		}
		else if (_state == RELAY_ON) { this->off(); }
		
		return _state;
	};

	void on() {
		digitalWrite(_pin, RELAY_ON);
		_state = RELAY_ON;
		saveState(_address, RELAY_ON);
		Serial.print("Relay: ");
		Serial.print(_address);
		Serial.println(" is ON");
		sendState();
	};

	void off() {
		digitalWrite(_pin, RELAY_OFF);
		_state = RELAY_OFF;
		saveState(_address, RELAY_OFF);
		Serial.print("Relay: ");
		Serial.print(_address);
		Serial.println(" is OFF");
		sendState();
	};

	bool getState() { return _state; };
	void sendState(bool ack = false) { 
		wait(20);
		if (send(_msg->set(_state ? RELAY_ON : RELAY_OFF), ack) == false)
		{
			wait(50);
			send(_msg->set(_state ? RELAY_ON : RELAY_OFF), ack);
		}

	}
	bool getAddr() { return _address; };

private:
	uint8_t _pin;
	uint8_t _address;
	bool _state;
	MyMessage* _msg;
};

class Channel {
public:
	Channel(uint8_t pin, uint8_t address) : _pin(pin), _address(address) {
		_value = 0;
		_state = LIGHT_OFF;
		_msg = new MyMessage(_address, V_PERCENTAGE);
	}
	void begin() {
		pinMode(_pin, OUTPUT);
		analogWrite(_pin, LIGHT_OFF);
		_value = loadState(_address);
		_state = (_value > 0);
		Serial.print("Channel: ");
		Serial.print(_address);
		Serial.print(" .Loading previous value: ");
		Serial.println(_value);
	}
	void sendState(bool ack = false) {
		float val;
		wait(20);
		_msg->setType(V_STATUS);
		
		if (send(_msg->set(_state), ack) == false )
		{
			wait(50);
			send(_msg->set(_state), ack);
		}

		wait(5);
		val = (_value / 255.) * 100.;
		_msg->setType(V_PERCENTAGE);
		Serial.print("Sending new value to gw: ");
		Serial.print(val);
		Serial.print("\n");

		if (send(_msg->set(static_cast<uint8_t>(val),0), ack) == false)
		{
			wait(50);
			send(_msg->set(static_cast<uint8_t>(val),0), ack);
		}
	}
	bool getState() { return _state; }
	
	void pwm(uint8_t value) { 
		if (_value != value) _value = value; 
		else return;
		if (_value == 0) _state = LIGHT_OFF;
		Serial.print("Channel: ");
		Serial.print(_address);
		Serial.println(" value is set to: ");
		Serial.print(_value);
		Serial.print("\n");
		analogWrite(_pin, _value);
		sendState();
		
		/*if (_value > 0)*/ saveState(_address, _value);
	}
	void on() { 
		_state = LIGHT_ON;
		pwm(loadState(_address)); 		
	}
	void off() {		
		pwm(0);
	}
	uint8_t getValue() { return _value; }
	bool getAddr() { return _address; };

private:
	uint8_t _pin;
	uint8_t _address;
	uint8_t _value;
	bool _state;
	MyMessage* _msg;	
};


bool timeReceived = false;
unsigned long lastRequest = 0;

Relay r1(RELAY1_PIN, CHILD_ID_RELAY1);
Relay r2(RELAY2_PIN, CHILD_ID_RELAY2);
Relay r3(RELAY3_PIN, CHILD_ID_RELAY3);
Relay r4(RELAY4_PIN, CHILD_ID_RELAY4);

Channel ch1(CHANNEL1_PIN, CHILD_ID_CHANNEL1);
Channel ch2(CHANNEL2_PIN, CHILD_ID_CHANNEL2);
Channel ch3(CHANNEL3_PIN, CHILD_ID_CHANNEL3);
Channel ch4(CHANNEL4_PIN, CHILD_ID_CHANNEL4);

OneWire oneWire(CHILD_ID_TEMP);
DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer;


float getTemp() {
	sensors.requestTemperatures();
	return DallasTemperature::toCelsius(sensors.getTempFByIndex(DS1820_INDEX));
}

void sendTemp() {
	MyMessage msg(CHILD_ID_TEMP, V_TEMP);
	send(msg.set(getTemp(), 1));
}

float getPH() {
	int measure = analogRead(CHILD_ID_PH);
	Serial.print("Measured value: ");
	Serial.print(measure);
	float pHVol = (5.0 / 1024) * measure;
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

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

void before()
{
	// Startup up the OneWire library
	//sensors.begin();
}

// the setup function runs once when you press reset or power the board
void setup() {
#ifndef MY_DEBUG
	Serial.begin(115200);
#endif // !MY_DEBUG

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

	// the function to get the time from the RTC
	//setSyncProvider(RTC.get);

	// Request latest time from controller at startup
	//requestTime();

	r1.begin();
	r2.begin();
	r3.begin();
	r4.begin();

	ch1.begin();
	ch2.begin();
	ch3.begin();
	ch4.begin();
}

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Aquas", "1.0",true);

	// Register all sensors to gw (they will be created as child devices)
	present(CHILD_ID_RELAY1, S_BINARY);
	present(CHILD_ID_RELAY2, S_BINARY);
	present(CHILD_ID_RELAY3, S_BINARY);
	present(CHILD_ID_RELAY4, S_BINARY);

	present(CHILD_ID_CHANNEL1, S_DIMMER);
	present(CHILD_ID_CHANNEL2, S_DIMMER);
	present(CHILD_ID_CHANNEL3, S_DIMMER);
	present(CHILD_ID_CHANNEL4, S_DIMMER);
	
	present(CHILD_ID_TEMP, S_WATER_QUALITY);
	present(CHILD_ID_PH, S_WATER_QUALITY);

}

// This is called when a new time value was received
/*void receiveTime(unsigned long controllerTime) {
	// Ok, set incoming time 
	Serial.print("Time value received: ");
	Serial.println(controllerTime);
	RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
	timeReceived = true;
}*/

void loop() {

	/*unsigned long now = millis();
	// If no time has been received yet, request it every 10 second from controller
	// When time has been received, request update every hour
	if ((!timeReceived && (now - lastRequest) > (10UL * 1000UL))
		|| (timeReceived && (now - lastRequest) > (60UL * 1000UL * 60UL))) {
		// Request time from controller. 
		Serial.println("requesting time");
		requestTime();
		lastRequest = now;
	}*/

	/*cur_time = elapsedSecsToday(RTC.get());*/

	sendTemp();
	sendPH();
	wait(60000);
	
	/*ch1.sendState();
	ch2.sendState();
	ch3.sendState();
	ch4.sendState();*/

	//sleep(10000, true);
	//sendHeartbeat();
}

void receive(const MyMessage &message) {
	bool prevState, newState;
	float newValue;

	if (message.isAck()) {
		Serial.println("This is an ack from gateway");
		return;
	}

	//if (message.sensor == MY_NODE_ID) { }
		if (message.type == V_STATUS) {

			Serial.print("\nIncoming change for sensor: ");
			Serial.print(message.sensor);
			Serial.print(", New status: ");
			Serial.println(message.getBool());

			switch (message.sensor) {

			case CHILD_ID_RELAY1: {
				newState = message.getBool();
				newState ? r1.on() : r1.off();
				break;
			}
			case CHILD_ID_RELAY2: {
				newState = message.getBool();
				newState ? r2.on() : r2.off();
				break;
			}
			case CHILD_ID_RELAY3: {
				newState = message.getBool();
				newState ? r3.on() : r3.off();
				break;
			}
			case CHILD_ID_RELAY4: {
				newState = message.getBool();
				newState ? r4.on() : r4.off();
				break;
			}
			case CHILD_ID_CHANNEL1: {
				newState = message.getBool();
				newState ? ch1.on() : ch1.off();
				break;
			}
			case CHILD_ID_CHANNEL2: {
				newState = message.getBool();
				newState ? ch2.on() : ch2.off();
				break;
			}
			case CHILD_ID_CHANNEL3: {
				newState = message.getBool();
				newState ? ch3.on() : ch3.off();
				break;
			}
			case CHILD_ID_CHANNEL4: {
				newState = message.getBool();
				newState ? ch4.on() : ch4.off();
				break;
			}

		}
	}
		if (message.type == V_PERCENTAGE) {
			Serial.print("\nIncoming change for channel: ");
			Serial.print(message.sensor);
			Serial.print(", New value: ");
			newValue = (message.getFloat() / 100) * 255;
			Serial.println(message.getUInt());

			switch (message.sensor) {
			case CHILD_ID_CHANNEL1: {
				ch1.pwm(newValue);
				break;
			}
			case CHILD_ID_CHANNEL2: {
				ch2.pwm(newValue);
				break;
			}
			case CHILD_ID_CHANNEL3: {
				ch3.pwm(newValue);
				break;
			}
			case CHILD_ID_CHANNEL4: {
				ch4.pwm(newValue);
				break;
			}

			}
		}
	
}
