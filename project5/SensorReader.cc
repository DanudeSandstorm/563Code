#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h> // For mmap_device_io()
#include <hw/inout.h> // For in / out functions

#include "SensorReader.h"

/**
 * This is written with the assumption that the ultrasonic sensor is connected
 * as follows:
 * 5V in  - Red    - Pin 27
 * GND    - Black  - Pin 28
 * Input  - Blue   - Pin 1
 * Output - Yellow - Pin 9
 */

// Data IO register addresses
#define BASE            (0x280)     // Base address for registers
#define DIO_A           (BASE + 8)  // Offset for Data IO A
#define DIO_B           (BASE + 9)  // Offset for Data IO B
#define DIO_CONFIG      (BASE + 11) // Configuration for Data IO

#define DIO_PIN_0       ((uint8_t) 0x01)   // Mask to access pin 0 on a Data IO port
#define DIO_A_IN_B_OUT  ((uint8_t) 0x10)   // Value to make DIO A an input and DIO B an output (0001 0000)
#define MIN_PULSE_DELAY (10 * 1000 * 1000) // 10ms minimum delay between pulses
#define SPEED_OF_SOUND  (1126 * 12)        // inches / second
#define MAX_DIST_PULSE_LENGTH (18000000)   // Max value is 18 ms

// Function prototypes
void sendStartPulse(void);
uint64_t readPulseLength(void);
double pulseToDistance(uint64_t);
void waitNs(uint32_t);
uint64_t timediffNs(struct timespec *, struct timespec *);


// Gives us access to the Data I/O pins
uintptr_t inputPin  = mmap_device_io(1, DIO_A);
uintptr_t outputPin = mmap_device_io(1, DIO_B);
uintptr_t configReg = mmap_device_io(1, DIO_CONFIG);

struct timespec lastReadingTime; // Stores end time of last reading

/**
 * Initializes sensor and data I/O ports
 */
void sensorInit() {
	out8(configReg, DIO_A_IN_B_OUT); // Configure DIO A as an input and DIO B as an output
	out8(outputPin, 0x00); // Hold our output low
}

/**
 * Wait until a safe amount of time has passed since the last measurement
 */
void sensorAwaitReady() {
	struct timespec now;
	do {
		clock_gettime(CLOCK_REALTIME, &now);
	} while (timespec2nsec(&now) < timespec2nsec(&lastReadingTime) + MIN_PULSE_DELAY);
}

/**
 * Find the distance the ultrasonic sensor is measuring
 */
double sensorReadDistance() {
	sendStartPulse();
	uint64_t val = readPulseLength();
	clock_gettime(CLOCK_REALTIME, &lastReadingTime);

	// Check for out of range measurements
	if (val > MAX_DIST_PULSE_LENGTH) {
		return OUT_OF_RANGE;
	}
	else {
		return pulseToDistance(val);
	}
}

// Block for X nanoseconds
void waitNs(uint32_t nsToWait) {
	struct timespec start;
	struct timespec curTime;
	clock_gettime(CLOCK_REALTIME, &start);

	do {
		clock_gettime(CLOCK_REALTIME, &curTime);
	} while (timespec2nsec(&curTime) <= timespec2nsec(&start) + nsToWait);

	return;
}

// Returns difference between two timespecs in nanoseconds
uint64_t timediffNs(struct timespec * start, struct timespec * end) {
	uint64_t startTimeNs = (uint64_t)(start->tv_sec * (uint64_t)1000000000) + start->tv_nsec;
	uint64_t endTimeNs   = (uint64_t)(end->tv_sec   * (uint64_t)1000000000) + end->tv_nsec;
	return endTimeNs - startTimeNs;
}

// Send initial pulse to the ultrasonic sensor to get a reading
void sendStartPulse() {
	// Input is held high for (at least) 10us to get a reading
	out8(outputPin, DIO_PIN_0); // Set output high
	waitNs(10 * 1000);          // Wait 10us
	out8(outputPin, 0x0);       // Set output low
}

// Read the echo from the ultrasonic sensor
uint64_t readPulseLength() {
	// Sensor starts sensing on falling edge (aka when we put output low again)
	struct timespec startTime;
	struct timespec endTime;

	// There is a delay while the sonic sensor emits a sound
	while( !(in8(inputPin) & DIO_PIN_0) );

	// Find the length of the pulse
	clock_gettime(CLOCK_REALTIME, &startTime);
	while(in8(inputPin) & DIO_PIN_0);
	clock_gettime(CLOCK_REALTIME, &endTime);

	uint64_t pulseLength = timediffNs(&startTime, &endTime);

	return pulseLength;
}

// Convert the ultrasonic sensor's return pulse to a distance
double pulseToDistance(uint64_t pulse) {
	// The pulse is the time between sound emission and receiving an echo.
	// The distance is therefore (pulse / 2) * speed of sound.
	// Pulse comes in measured in nanoseconds.

	uint64_t distanceNanoInch = (pulse / 2) * SPEED_OF_SOUND;
	double distanceNormalInch = static_cast<double>(distanceNanoInch) / 1e9;

	return distanceNormalInch;
}
