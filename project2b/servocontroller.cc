#include <pthread.h>  // For threads
#include <sys/mman.h> // For mmap_device_io()
#include <hw/inout.h> // For in / out functions

#include "servocontroller.h"
#include "pwm.h"

/* Data IO Constants */
#define BASE            (0x280)            // Base address for registers
#define DIO_A           (BASE + 8)         // Offset for Data IO A
#define DIO_CONFIG      (BASE + 11)        // Configuration for Data IO
#define DIO_PIN_0       ((uint8_t) 0x01)   // Mask to access pin 0 on Data IO port
#define DIO_PIN_1       ((uint8_t) 0x02)   // Make to access pin 1 on Data IO port
#define DIO_A_OUT       ((uint8_t) 0x00)   // Value to make DIO A an output (0000 0000)

const uint32_t SERVO_CYCLE_US = 20000;

// Values to convert servo position to a duty cycle
static const double DUTY_POS[6] = {
		0.04,   // 0
		0.055,  // 1
		0.0725, // 2
		0.085,  // 3
		0.10,   // 4
		0.12    // 5
};

// Gives us access to the Data I/O pins
uintptr_t pwmPins;
uintptr_t configReg;

uint8_t dioAValue;         // Track value of DIO A, since we can't read it while it's in output mode
pthread_mutex_t dioAMutex; // We need to be able to lock the Data IO port to be thread safe

/* Helper function prototypes */
ServoStuff* selectServo(ServoController* instance, uint8_t servoid);
void servo1On();
void servo1Off();
void servo2On();
void servo2Off();

/* Member functions */
ServoController::ServoController() {
	// Initialize servo controller values
	this->servo1.pwm.setCycle(SERVO_CYCLE_US);
	this->servo1.pwm.setRisingEdgeEvent(servo1On);
	this->servo1.pwm.setFallingEdgeEvent(servo1Off);

	this->servo2.pwm.setCycle(SERVO_CYCLE_US);
	this->servo2.pwm.setRisingEdgeEvent(servo2On);
	this->servo2.pwm.setFallingEdgeEvent(servo2Off);
}

// Starts threads for PWM controllers
void ServoController::startServos() {
	// Set up Data IO port
	pwmPins   = mmap_device_io(1, DIO_A);
	configReg = mmap_device_io(1, DIO_CONFIG);
	uintptr_t resetReg = mmap_device_io(1, 0x280);

	out8(resetReg, 0x64);

	dioAValue = 0x00;
	out8(configReg, DIO_A_OUT);

	pthread_create(
		&(this->servo1.thread),
		NULL,
		&PWMController::run_helper,
		&(this->servo1.pwm)
	);

	pthread_create(
		&(this->servo2.thread),
		NULL,
		&PWMController::run_helper,
		&(this->servo2.pwm)
	);
	
	// Default our servo positions
	this->setServoPosition(1, 0);
	this->setServoPosition(2, 0);
}

// Stops and joins threads for PWM controllers
void ServoController::stopServos() {
	pthread_join(this->servo1.thread, NULL);
	pthread_join(this->servo2.thread, NULL);
}

// Move servo to position, checking for invalid positions
uint8_t ServoController::setServoPosition(uint8_t servoId, uint8_t position) {
	// Don't allow invalid servo positions
	if (position < 0 || 5 < position) {
		return SERVO_ERROR;
	}

	ServoStuff* selectedServo = this->selectServo(servoId);
	
	selectedServo->position = position;
	selectedServo->pwm.setDuty(DUTY_POS[position]);
	return 0;
}

// Move servo up one position, checking for invalid positions
uint8_t ServoController::servoUp(uint8_t servoId) {
	ServoStuff* selectedServo = this->selectServo(servoId);
	
	if (selectedServo->position < 5) {
		selectedServo->position++;
		selectedServo->pwm.setDuty(DUTY_POS[selectedServo->position]);
		return 0;
	}
	else {
		return SERVO_ERROR;
	}
}

// Move servo down one position, checking for invalid positions
uint8_t ServoController::servoDown(uint8_t servoId) {
	ServoStuff* selectedServo = this->selectServo(servoId);
	
	if (selectedServo->position > 0) {
		selectedServo->position--;
		selectedServo->pwm.setDuty(DUTY_POS[selectedServo->position]);
		return 0;
	}
	else {
		return SERVO_ERROR;
	}
}

ServoStuff* ServoController::selectServo(uint8_t servoId) {
	// Convert a servoId into a servo controller object
	if (servoId == 1) {
		return &(this->servo1);
	} else if (servoId == 2) {
		return &(this->servo2);
	} else {
		return NULL;
	}
}

/* Helper functions */

void servo1On() {
	pthread_mutex_lock(&dioAMutex);
	dioAValue |= DIO_PIN_0;
	out8(pwmPins, dioAValue);
	pthread_mutex_unlock(&dioAMutex);
}

void servo1Off() {
	pthread_mutex_lock(&dioAMutex);
	dioAValue &= ~DIO_PIN_0;
	out8(pwmPins, dioAValue);
	pthread_mutex_unlock(&dioAMutex);
}

void servo2On() {
	pthread_mutex_lock(&dioAMutex);
	dioAValue |= DIO_PIN_1;
	out8(pwmPins, dioAValue);
	pthread_mutex_unlock(&dioAMutex);
}

void servo2Off(){
	pthread_mutex_lock(&dioAMutex);
	dioAValue &= ~DIO_PIN_1;
	out8(pwmPins, dioAValue);
	pthread_mutex_unlock(&dioAMutex);
}
