/*
 Name:		Sketch1.ino
 Created:	12/20/2015 8:47:26 AM
 Author:	admin
 */
// Node ID
#define MY_NODE_ID 51

// Enable debug prints to serial monitor
#define MY_DEBUG 

//#define MY_DEBUG_VERBOSE_RF24

// Enable and select radio type attached
#define MY_RADIO_NRF24
 //#define MY_RADIO_RFM69

#define MY_RF24_PA_LEVEL RF24_PA_MAX

// For Mega2560
#define MY_RF24_CE_PIN 49
#define MY_RF24_CS_PIN 53

#define INVERSE_LOGIC

 // Enabled repeater feature for this node
 //#define MY_REPEATER_FEATURE

#include <MySensors.h>
#include <MyConfig.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <Wire.h>

// Digital pins
#define RELAY1_PIN 7  // Light
#define RELAY2_PIN 8  // CO2
#define RELAY3_PIN 9 // Filter
#define RELAY4_PIN 4 // Heater


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


long filter_time, co2_time, oxygen_time; //variables for starting time
long filter_duration, co2_duration, oxygen_duration; //duration variables

bool timeReceived = false;
unsigned long lastRequest = 0;

Relay r1(RELAY1_PIN, CHILD_ID_RELAY1);
Relay r2(RELAY2_PIN, CHILD_ID_RELAY2);
Relay r3(RELAY3_PIN, CHILD_ID_RELAY3);
Relay r4(RELAY4_PIN, CHILD_ID_RELAY4);


// the setup function runs once when you press reset or power the board
void setup() {
#ifndef MY_DEBUG
	Serial.begin(115200);
#endif // !MY_DEBUG

	// the function to get the time from the RTC
	setSyncProvider(RTC.get);

	// Request latest time from controller at startup
	requestTime();

	// sunset params
	/*rise_start = 7 * SECS_PER_HOUR + 15 * SECS_PER_MIN;
	set_start = 16 * SECS_PER_HOUR + 45 * SECS_PER_MIN; */

	// small aquas
	/*filter_time = 12 * SECS_PER_HOUR + 30 * SECS_PER_MIN; // 10:30 AM
	co2_time = 12 * SECS_PER_HOUR + 20 * SECS_PER_MIN;;
	oxygen_time = 21 * SECS_PER_HOUR; //21:00

	filter_duration = 8 * SECS_PER_HOUR + 0 * SECS_PER_MIN; // 8 hours
	co2_duration = 9 * SECS_PER_HOUR;
	oxygen_duration = 2 * SECS_PER_HOUR; //2 hours*/

	// big aquas
	filter_time = 7 * SECS_PER_HOUR + 0 * SECS_PER_MIN;
	co2_time = 7 * SECS_PER_HOUR + 0 * SECS_PER_MIN;
	oxygen_time = 7 * SECS_PER_HOUR + 45 * SECS_PER_MIN;

	filter_duration = 10 * SECS_PER_HOUR + 30 * SECS_PER_MIN;
	co2_duration = 10 * SECS_PER_HOUR;
	oxygen_duration = 10 * SECS_PER_HOUR + 0 * SECS_PER_MIN;

	r1.begin();
	r2.begin();
	r3.begin();
	r4.begin();

	/*
	// init relay pins
	pinMode(RELAY4_PIN, OUTPUT);
	pinMode(RELAY3_PIN, OUTPUT);
	pinMode(RELAY1_PIN, OUTPUT);
	pinMode(RELAY2_PIN, OUTPUT);


	// Make sure relays are off when starting up
	digitalWrite(RELAY1_PIN, RELAY_OFF);
	digitalWrite(RELAY2_PIN, RELAY_OFF);
	digitalWrite(RELAY3_PIN, RELAY_OFF);
	digitalWrite(RELAY4_PIN, RELAY_OFF);
	*/
	// Init PWM for leds
	/*analogWrite(_RED_PIN, 0);
	analogWrite(_BLUE_PIN, 0);
	analogWrite(_WHITE_PIN, 0);*/

	//// Set relays to last known state (using eeprom storage) 
	//state1 = loadState(CHILD_ID_RELAY1);
	//digitalWrite(RELAY1_PIN, state1 ? RELAY_ON : RELAY_OFF);
	//state2 = loadState(CHILD_ID_RELAY2);
	//digitalWrite(RELAY2_PIN, state2 ? RELAY_ON : RELAY_OFF);
	//state3 = loadState(CHILD_ID_RELAY3);
	//digitalWrite(RELAY3_PIN, state3 ? RELAY_ON : RELAY_OFF);
	//state4 = loadState(CHILD_ID_RELAY4);
	//digitalWrite(RELAY4_PIN, state4 ? RELAY_ON : RELAY_OFF);

}

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Aquas", "1.0",true);

	// Register all sensors to gw (they will be created as child devices)
	present(CHILD_ID_RELAY1, S_BINARY);
	present(CHILD_ID_RELAY2, S_BINARY);
	present(CHILD_ID_RELAY3, S_BINARY);
	present(CHILD_ID_RELAY4, S_BINARY);

	/*r1.sendState(false);
	wait(25);
	r2.sendState(false);
	wait(25);
	r3.sendState(false);
	wait(25);
	r4.sendState(false);
	wait(25);*/
}

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
	// Ok, set incoming time 
	Serial.print("Time value received: ");
	Serial.println(controllerTime);
	RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
	timeReceived = true;
}



// the loop function runs over and over again until power down or reset
void loop() {

	unsigned long now = millis();
	// If no time has been received yet, request it every 10 second from controller
	// When time has been received, request update every hour
	if ((!timeReceived && (now - lastRequest) > (10UL * 1000UL))
		|| (timeReceived && (now - lastRequest) > (60UL * 1000UL * 60UL))) {
		// Request time from controller. 
		Serial.println("requesting time");
		requestTime();
		lastRequest = now;
	}

	cur_time = elapsedSecsToday(RTC.get());
	/*r1.proceed(filter_time, filter_duration);
	r2.proceed(co2_time, co2_duration);
	r3.proceed(oxygen_time, oxygen_duration);
	r4.proceed(oxygen_time, oxygen_duration);*/
	
	wait(1000);
	//sendHeartbeat();
}

void receive(const MyMessage &message) {
	bool prevState, newState;
	// We only expect one type of message from controller. But we better check anyway.
	/*if (message.isAck()) {
	Serial.println("This is an ack from gateway");
	return;
	}
	*/
	if (message.sensor == MY_NODE_ID) {}
	if (message.type == V_STATUS) {
		// Write some debug info
		Serial.print("\nIncoming change for sensor: ");
		Serial.print(message.sensor);
		Serial.print(", New status: ");
#ifdef INVERSE_LOGIC
		Serial.println(!message.getBool());
#else
		Serial.println(message.getBool());
#endif
		switch (message.sensor) {

		case CHILD_ID_RELAY1: {
			prevState = r1.getState();
#ifdef INVERSE_LOGIC
			prevState = !prevState;
#endif	
			newState = message.getBool();
			// Change relay state
			//if (newState != prevState) {
				newState ? r1.on() : r1.off();
			//}
			break;
		}
		case CHILD_ID_RELAY2: {
			prevState = r2.getState();
#ifdef INVERSE_LOGIC
			prevState = !prevState;
#endif
			newState = message.getBool();
			// Change relay state
			//if (newState != prevState) {
				newState ? r2.on() : r2.off();
			//}
			break;
		}
		case CHILD_ID_RELAY3: {
			prevState = r3.getState();
#ifdef INVERSE_LOGIC
			prevState = !prevState;
#endif
			newState = message.getBool();
			// Change relay state
			//if (newState != prevState) {
				newState ? r3.on() : r3.off();
			//}
			break;
		}
		case CHILD_ID_RELAY4: {
			prevState = r4.getState();
#ifdef INVERSE_LOGIC
			prevState = !prevState;
#endif
			newState = message.getBool();
			// Change relay state
			//if (newState != prevState) {
				newState ? r4.on() : r4.off();
			//}
			break;
		}

		}		
	}
}