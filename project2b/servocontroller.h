#ifndef servocontroller_h
#define servocontroller_h

#include <pthread.h> // For threads and mutexes
#include <stdint.h>
#include "pwm.h"

#define SERVO_ERROR (1)

struct ServoStuff {
	PWMController pwm;
	pthread_t thread;
	uint8_t position;
};

class ServoController {

	private:
		ServoStuff servo1;
		ServoStuff servo2;
		ServoStuff* selectServo(uint8_t servoId);

	public:
		ServoController(void);

		void startServos(void);
		void stopServos(void);
		uint8_t setServoPosition(uint8_t servoId, uint8_t position);
		uint8_t servoUp(uint8_t servoId);
		uint8_t servoDown(uint8_t servoId);

};

#endif
