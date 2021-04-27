#include "config.h"

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
		Serial.print(" loading previous state: ");
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
	MyMessage *_msg;	
};
