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
 * thread. Also note: it uses rand(), which is apparently not thread safe so we'll
 * need to find some thread safe way to generate random intervals. Could just wrap
 * rand() in our own safeRand() function with a mutex (like the queue)
 */
void * customerCreator(void * arg){
	srand (time(NULL));
	std::cout << "Customer Creator thread created" << std::endl;
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

/**
 * logic for teller thread. Should probably pass in the teller id as an arg, so they can print their id with
 * their messages and we can tell them apart
 */
void * teller(void * arg){
	std::cout << "Teller Created" << std::endl;
	while(running || !customerQueue.empty()){
		int curCustomer = dequeueCustomer();
		if(curCustomer != -1){
			std::cout << "Serving Customer" << std::endl;
			scaledSleep(360);	//TODO: sleep for rand val between 30 sec and 6 minutes, but rand() needs to be made thread safe
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
	while(running){
		//if the main thread exits, all other threads it spawned are also killed
	}
	//when day has ended, wait for all tellers to clear the queue and finish their customers
	for(int i = 0; i < NUM_TELLERS; i++){
		pthread_join(tellerThread[i], NULL);
	}
	return EXIT_SUCCESS;
}

