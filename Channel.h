#include "config.h"

#define LIGHT_OFF 0
#define LIGHT_ON 1

#define DESC_SIZE 25

class Channel {
public:
	Channel(uint8_t pin, uint8_t address, const char *description) : _pin(pin), _address(address) {
		_value = 0;
		_state = LIGHT_OFF;
		_msg = new MyMessage(_address, V_PERCENTAGE);
		_description[DESC_SIZE] = {0};
    	strncpy(_description, description, sizeof(_description) - 1);
    	_description[sizeof(_description) - 1] = '\0';
	}
	void begin() {
		pinMode(_pin, OUTPUT);
		analogWrite(_pin, LIGHT_OFF);
		_value = loadState(_address);
		_state = (_value > 0);
		Serial.print("Channel: ");
		Serial.print(_address);
		Serial.print(" loading previous value: ");
		Serial.println(_value);
	}
	void sendState(bool ack = false) {
		float val;
		wait(20);
		_msg->setType(V_STATUS);

		if (send(_msg->set(_state), ack) == false)
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

		if (send(_msg->set(static_cast<uint8_t>(val), 0), ack) == false)
		{
			wait(50);
			send(_msg->set(static_cast<uint8_t>(val), 0), ack);
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
	void setDescr(const char *description) { 
    	strncpy(_description, description, sizeof(_description) - 1);
    	_description[sizeof(_description) - 1] = '\0';
    }
	char * getDescr() { return _description; } 


private:
	uint8_t _pin;
	uint8_t _address;
	uint8_t _value;
	bool _state;
	MyMessage* _msg;
	char _description[DESC_SIZE];
};
