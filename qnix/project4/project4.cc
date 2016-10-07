#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <time.h>
#include <unistd.h>


bool running = true;
int customerId = 0;
std::queue<int> customerQueue;
pthread_t * custCreatorThread;
pthread_attr_t custCreatorAttr;

/**
 * Takes an amount of time to sleep in simulated seconds, and performs
 * the correct sleep in real time.
 * 100ms real time == 1 sec simulation time
 * 1 sec real time == 10 sec simulation time
 */
void scaledSleep(int simSleepTime){
	int realSleepTime = simSleepTime / 10;
	sleep(realSleepTime);
}

void spawnNewCustomer(void){
	customerQueue.push(customerId);
	customerId++;
	std::cout << "New Customer Added" << std::endl;
}

int dequeueCustomer(void) {
	if(!customerQueue.empty()){
		std::cout << "Customer Serviced" << std::endl;
		int result = customerQueue.front();
		customerQueue.pop();
		return result;
	}
	return -1;
}

/**
 * This method waits a random interval and then creates a new customer, looping
 * until !running. This should only be used in the initialization of the customerCreator
 * thread. Also note: it uses rand(), which is apparently not thread safe so we'll
 * need to find some thread safe way to generate random intervals
 */
void * customerCreator(void * arg){
	srand (time(NULL));
	std::cout << "Thread created" << std::endl;
	while(running){
		//get random interval to wait for next customer
		int interval = (rand() % 180) + 60;	//customers arrive every 1 to 4 minutes
		//wait for next customer
		scaledSleep(interval);
		//create new customer
		spawnNewCustomer();
	}
	return arg;
}

int main(int argc, char *argv[]) {
	pthread_create(custCreatorThread, &custCreatorAttr, &customerCreator, '\0');


	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	while(running){
		//if the main thread exits, all other threads it spawned are also killed
	}
	return EXIT_SUCCESS;
}

