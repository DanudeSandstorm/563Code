#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include <unistd.h>       // For sleep
#include <stdio.h>
#include <sys/neutrino.h> // For ThreadCtl and ClockPeriod
#include <pthread.h>

#include "SensorReader.h"

#define MIN_CLOCK_RESOLUTION (1000 * 10) // 10us

void display_value(float value);
void print_results(void);
void setMinMax(uint32_t value);
uint32_t round(double value);
void *key_listen( void *ptr );


uint32_t maxValue;
uint32_t minValue;

static const float minRange = 1.18; //3 cm
static const float maxRange = 118.11; //3 meters

volatile bool sensorRead = false;


int main(int argc, char *argv[]) {
	std::cout << "Press s to begin and f to finish" << std::endl;

	// Get root permissions to access I/O ports
	int privityErr = ThreadCtl(_NTO_TCTL_IO, NULL);
	if (privityErr == -1) {
		std::cout << "Could not get root access for I/O" << std::endl;
		return EXIT_FAILURE;
	}

	// Set the resolution of our clock higher for more accurate distance measurements.
	struct _clockperiod newRes;
	newRes.fract = 0; // This is always 0
	newRes.nsec = MIN_CLOCK_RESOLUTION;
	ClockPeriod(CLOCK_REALTIME, &newRes, NULL, 0);

	sensorInit();

	// Set up start/stop key thread
	int keyListener;
	pthread_t thread0;
	keyListener = pthread_create( &thread0, NULL, key_listen, (void*) NULL);

	// Don't start until start key is pressed
	while(!sensorRead){}

	std::cout << "Beginning measurements" << std::endl;

	while (sensorRead) {
		sensorAwaitReady();
		double dist = sensorReadDistance();

		if (dist == OUT_OF_RANGE) {
			std::cout << "Dist: *" << std::endl;
		}
		else {
			uint32_t roundedDist = round(dist);
			setMinMax(roundedDist);
			std::cout << "Dist: " << roundedDist << std::endl;
			usleep(1e5); // sleep for 100ms
		}
	}

	pthread_join(keyListener, NULL);

	std::cout << std::endl << "Stopped" << std::endl << std::endl;
	std::cout << "Max Value: " << maxValue << std::endl;
	std::cout << "Min Value: " << minValue << std::endl;

	return EXIT_SUCCESS;
}

uint32_t round(double value) {
	uint32_t intValue = static_cast<uint32_t>(value);
	if (static_cast<uint32_t>(value * 10) % 10 >= 5) {
		intValue += 1; 
	}
	return intValue;
}

void display_value(float value){
	if (value >= minRange and value <= maxRange) {
		//round to the nearest integer value
		int intValue = (int) value;
		if (((int)(value * 10) % 10) >= 5){
			intValue++;
		}

		setMinMax(intValue);

		std::cout << "Dist: " << intValue; //print the result
	} else {
		std::cout << "Dist: *";
	}

}

void setMinMax(uint32_t value) {
	//determine if the value is new min or max
	if (!maxValue || value > maxValue) {
		maxValue = value;
	}

	if (!minValue || value < minValue) {
		minValue = value;
	}
}

void print_results(){
	printf("Max Value %d\n", maxValue);
	printf("Min Value %d\n", minValue);
}


void *key_listen( void *ptr ){
	char c = getchar();
	while (c != EOF){
		if (c == 's') {
			sensorRead = true;
		}
		if (c == 'f'){
			sensorRead = false;
		}
		c = getchar();
	}
	pthread_exit(NULL);
}
