#ifndef SensorReader_h
#define SensorReader_h

#define OUT_OF_RANGE ((double) -1.0)

/**
 * Initializes sensor and data I/O
 */
void sensorInit(void);

/**
 * Wait until a safe amount of time has passed since the last measurement.
 *
 * The ultrasonic sensor's spec sheet mentions there should be a delay
 * between reading measurements. This function blocks the calling thread
 * until that amount of time has passed.
 */
void sensorAwaitReady(void);

/**
 * Reads the current distance from the ultrasonic sensor.
 *
 * Blocks until it gets a reading. No guarantee on the duration of the block,
 * but it will probably be between 100 microseconds and 10 milliseconds.
 */
double sensorReadDistance(void);

#endif

