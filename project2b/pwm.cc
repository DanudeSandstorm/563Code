#include <time.h>
#include <stdint.h>
#include <hw/inout.h>
#include <unistd.h>

#include "pwm.h"

// Helper function prototypes
void waitNs(uint32_t nsToWait);

/* PWMController Member Functions */

/**
 * Emulates a continuous PWM cycle.
 * Runs an optional, user-defined event on the rising and falling edges
 * of the pulse.
 *
 * This is a BLOCKING CALL, so it should be run in a thread.
 * Use PWMController::stop() to break out of this routine.
 */
void PWMController::run() {
	this->running = true;

	// Number of microseconds to hold signal high
	uint32_t highTimeUs;
	uint32_t lowTimeUs;

	while (this->running) {
		// Dynamically recalculate these so we can change the duty cycle on the fly
		highTimeUs = static_cast<uint32_t>(this->pulsePeriodUs * this->dutyCycle);
		lowTimeUs  = static_cast<uint32_t>(this->pulsePeriodUs - highTimeUs);

		if (this->onRisingEdge != NULL) onRisingEdge();
		waitNs(highTimeUs * 1000);
		if (this->onFallingEdge != NULL) onFallingEdge();
		waitNs(lowTimeUs * 1000);
	}
}

void PWMController::setCycle(uint32_t cycle) {
	this->pulsePeriodUs = cycle;
}

void PWMController::setDuty(double duty) {
	this->dutyCycle = duty;
}

void PWMController::stop() {
	this->running = false;
}

void PWMController::setFallingEdgeEvent(void (*fallingEdgeFunction)()) {
	this->onFallingEdge = fallingEdgeFunction;
}

void PWMController::setRisingEdgeEvent(void (*risingEdgeFunction)()) {
	this->onRisingEdge = risingEdgeFunction;
}

/* Helper Functions */

void waitNs(uint32_t nsToWait) {
	/* Block for X nanoseconds */
	struct timespec start;
	struct timespec curTime;
	clock_gettime(CLOCK_REALTIME, &start);

	do {
		clock_gettime(CLOCK_REALTIME, &curTime);
	} while (timespec2nsec(&curTime) <= timespec2nsec(&start) + nsToWait);

	return;
}

