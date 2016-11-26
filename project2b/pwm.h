#ifndef PWM_H
#define PWM_H

#include <stdint.h>

class PWMController {
	private:
		uint32_t pulsePeriodUs;  // Pulse period in microseconds
		double dutyCycle;        // Duty cycle (as a fraction)
		bool running;            // Flag that allows us to stop the PWM cycle across threads
		void (*onFallingEdge)(); // Pointer to function to run on falling clock edge
		void (*onRisingEdge)();  // Pointer to function to run on riding clock edge

	public:
		void setCycle(uint32_t cycle);
		void setDuty(double duty);
		void setFallingEdgeEvent(void (*fallingEdgeFunction)());
		void setRisingEdgeEvent(void (*risingEdgeFunction)());
		void run(void);
		void stop(void);

		/**
		 * We need this static function so we can use run() in a thread.
		 * The function is passed as PWMController::run_helper,
		 * and the parameter is a reference to a PWMController object.
		 */
		static void *run_helper(void *context) {
			((PWMController *)context)->run();
			return NULL;
		}
};

#endif
