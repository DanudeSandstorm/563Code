#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_TELLERS 3

bool running = true;
int customerId = 0;
std::queue<int> customerQueue;
pthread_mutex_t queueMutex;
pthread_t * custCreatorThread;
pthread_attr_t custCreatorAttr;
pthread_t tellerThread[NUM_TELLERS];
pthread_attr_t tellerAttr[NUM_TELLERS];

int simStartTime = time(NULL);
int getSimulationTime(){

}

/**
 * Takes an amount of time to sleep in simulated seconds, and performs
 * the correct sleep in real time.
 * 50ms real time == 30sec simulation time
 * 100ms real time == 1 minute simulation time
 * 1 sec real time == 10 minute simulation time
 */
void scaledSleep(int simSleepTime){
	int realSleepTime = simSleepTime * 1666;
	usleep(realSleepTime);
}

/**
 * rand() is apparently not necessarily thread safe, so we'll wrap it
 * in a mutex just to make sure
 */
pthread_mutex_t randMutex;
int safeRand(){
	pthread_mutex_lock( &randMutex );
	int result = rand();
	pthread_mutex_unlock(&randMutex);
	return result;
}

/**
 * returns a random int between min and max
 */
int safeRandInterval(int min, int max){
	//sanity check, min must be less than max
	if(min > max){
		int swap = min;
		min = max;
		max = swap;
	}
	return (safeRand() % (max - min)) + min;
}

void spawnNewCustomer(void){
	pthread_mutex_lock( &queueMutex );
	customerQueue.push(customerId);
	customerId++;
	pthread_mutex_unlock(&queueMutex);
	std::cout << "New Customer Added" << std::endl;
}

int dequeueCustomer(void) {
	if(!customerQueue.empty()){
		pthread_mutex_lock( &queueMutex );
		int result = customerQueue.front();
		customerQueue.pop();
		pthread_mutex_unlock(&queueMutex);
		return result;
	}
	return -1;
}

/**
 * This method waits a random interval and then creates a new customer, looping
 * until !running. This should only be used in the initialization of the customerCreator
 * thread.
 */
void * customerCreator(void * arg){
	srand (time(NULL));
	std::cout << "Customer Creator thread created" << std::endl;
	while(running){
		//create new customer
		spawnNewCustomer();
		//get random interval to wait for next customer
		int interval = safeRandInterval(60, 60*4);	//customers arrive every 1 to 4 minutes
		//wait for next customer
		scaledSleep(interval);
	}
	return arg;
}

/**
 * logic for teller thread. Should probably pass in the teller id as an arg, so they can print their id with
 * their messages and we can tell them apart
 */
void * teller(void * arg){
	std::cout << "Teller Created" << std::endl;
	while(running || !customerQueue.empty()){
		int curCustomer = dequeueCustomer();
		if(curCustomer != -1){
			std::cout << "Serving Customer" << std::endl;	//TODO: add customer id and teller id to printlns
			int interval = safeRandInterval(30, 60*6);	//sleep for rand val between 30 sec and 6 minutes
			scaledSleep(interval);
			std::cout << "Finished with customer" << std::endl;
		}
	}
	return arg;
}

int main(int argc, char *argv[]) {
	pthread_create(custCreatorThread, &custCreatorAttr, &customerCreator, '\0');

	for(int i = 0; i < NUM_TELLERS; i++){
		pthread_create(&tellerThread[i], &tellerAttr[i], &teller, '\0');
	}

	std::cout << "Welcome to the the bank, MOTHERFUCKER" << std::endl;

	sleep((7*60)/10);	//sleep for the duration of the day
	running = false;	//tell threads to stop

	std::cout << "Bank is closed" << std::endl;

	//when day has ended, wait for all tellers to clear the queue and finish their customers
	for(int i = 0; i < NUM_TELLERS; i++){
		pthread_join(tellerThread[i], NULL);
	}
	return EXIT_SUCCESS;
}

