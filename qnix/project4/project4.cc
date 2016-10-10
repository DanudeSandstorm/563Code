#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#define NUM_TELLERS 3

typedef struct {
	int totalCustomers;       // the total number of customers serviced during the day
	float avgCustWait;        // the average time each customer spends waiting in the queue
	float avgTellerWait;      // the average time tellers wait for customers
	float avgTransactionTime; // the average time each customer spends with the teller
	
	int longestLine;          // the maximum depth of the queue
	int maxCustWait;          // the maximum customer wait time in the queue
	int maxTellerWait;        // the maximum wait time for tellers waiting for customers
	int maxTransactionTime;   // the maximum transaction time for the tellers
} Metrics;

typedef struct {
	int id;
	int timeArrived;
	int timeStarted;
	int timeFinished;
} Customer;


bool running = true;
struct timespec simStartTime;

std::queue<Customer> customerQueue;
pthread_mutex_t queueMutex;

Metrics metrics;
pthread_mutex_t metricMutex;

// Customer creation thread
pthread_t custCreatorThread;
pthread_attr_t custCreatorAttr;

// Teller threads
pthread_t tellerThread[NUM_TELLERS];
pthread_attr_t tellerAttr[NUM_TELLERS];

void updateMetrics(Customer * c, int tellerWait){
	int custWait = c->timeStarted - c->timeArrived;
	int transactionTime = c->timeFinished - c->timeStarted;

	pthread_mutex_lock( &metricMutex );

	metrics.avgCustWait = ((metrics.avgCustWait * metrics.totalCustomers) + custWait) / (metrics.totalCustomers + 1);
	metrics.avgTransactionTime = ((metrics.avgTransactionTime * metrics.totalCustomers) + transactionTime) / (metrics.totalCustomers + 1);
	metrics.avgTellerWait = ((metrics.avgTellerWait * metrics.totalCustomers) + tellerWait) / (metrics.totalCustomers + 1);
	if(custWait > metrics.maxCustWait){
		metrics.maxCustWait = custWait;
	}
	if(transactionTime > metrics.maxTransactionTime){
		metrics.maxTransactionTime = transactionTime;
	}
	if(tellerWait > metrics.maxTellerWait){
		metrics.maxTellerWait = tellerWait;
	}
	metrics.totalCustomers = metrics.totalCustomers + 1;

	pthread_mutex_unlock(&metricMutex);
}

/* Returns the number of milliseconds since the program started */
int currentTime() {
	struct timespec curTime;
	clock_gettime(CLOCK_REALTIME, &curTime);

	int deltaSeconds = curTime.tv_sec - simStartTime.tv_sec;
	int deltaMilliseconds = (curTime.tv_nsec - simStartTime.tv_nsec) / 1000;

	return (deltaSeconds * 1000) + deltaMilliseconds;
}

/**
 * Takes an amount of time to sleep in simulated seconds, and performs
 * the correct sleep in real time.
 * 50ms real time == 30sec simulation time
 * 100ms real time == 1 minute simulation time
 * 1 sec real time == 10 minute simulation time
 */
void scaledSleep(int simSleepTime) {
	// 1666 is the conversion between microseconds and 100 ms
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
	static int customerId = 1;

	// Create a new customer
	Customer newCust;
	newCust.id = customerId;
	newCust.timeArrived = currentTime();

	// Get a lock on the customer queue and push that new customer into it
	pthread_mutex_lock( &queueMutex );
	customerQueue.push(newCust);

	// Is this the longest line so far?
	pthread_mutex_lock( &metricMutex );
	if (metrics.longestLine < customerQueue.size()) {
		metrics.longestLine = customerQueue.size();
	}
	pthread_mutex_unlock( &metricMutex );

	pthread_mutex_unlock(&queueMutex);

	customerId++;
	metrics.totalCustomers++;

	std::cout << "Customer " << newCust.id << " entered the bank." << std::endl;
}


Customer * dequeueCustomer(void) {
	Customer * custPtr = NULL;

	pthread_mutex_lock( &queueMutex );
	if(!customerQueue.empty()) {
		custPtr = &customerQueue.front();
		customerQueue.pop();
	}
	pthread_mutex_unlock(&queueMutex);

	return custPtr;
}

/**
 * This method waits a random interval and then creates a new customer, looping
 * until !running. This should only be used in the initialization of the customerCreator
 * thread.
 */
void * customerCreator(void * arg){
	std::cout << "Customer Creator thread created." << std::endl;
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
	int tellerId = *((int *)arg);

	int lastServiceEndTime = currentTime();
	std::cout << "Teller " << tellerId << " is open." << std::endl;
	while(running || !customerQueue.empty()){
		Customer * curCustomer = dequeueCustomer();
		if(curCustomer != NULL){
			std::cout << "Teller " << tellerId << " serving Customer " << curCustomer->id << std::endl;
			int interval = safeRandInterval(30, 60*6);	//sleep for rand val between 30 sec and 6 minutes
			curCustomer->timeStarted = currentTime();
			scaledSleep(interval);
			curCustomer->timeFinished = currentTime();
			updateMetrics(curCustomer, curCustomer->timeFinished - lastServiceEndTime);
			lastServiceEndTime = curCustomer->timeFinished;
			std::cout << "Teller " << tellerId << " finished with customer " << curCustomer->id << std::endl;
		}
	}
	std::cout << "Teller " << tellerId << " is done for the day" << std::endl;
	return arg;
}

int main(int argc, char *argv[]) {
	// Initialize our RNG and time stuff
	srand(time(NULL));
	clock_gettime(CLOCK_REALTIME, &simStartTime);


	pthread_create(&custCreatorThread, &custCreatorAttr, &customerCreator, '\0');

	int ids[3]; // So we can give teller thread its ID
	for(int i = 0; i < NUM_TELLERS; i++) {
		ids[i] = i + 1;
		pthread_create(&tellerThread[i], &tellerAttr[i], &teller, &ids[i]);
	}

	std::cout << "Welcome to the the bank, MOTHERFUCKER." << std::endl;

	sleep((7*60)/10);	//sleep for the duration of the day
	running = false;	//tell threads to stop

	std::cout << "Bank is closed." << std::endl;

	//when day has ended, wait for all tellers to clear the queue and finish their customers
	for(int i = 0; i < NUM_TELLERS; i++){
		pthread_join(tellerThread[i], NULL);
	}

	std::cout << "All customers have been served." << std::endl;

	// Now output our metrics information
	std::cout << std::endl << "BANK METRICS:" << std::endl;
	std::cout << "Customers served:           " << metrics.totalCustomers     << std::endl;
	std::cout << "Longest line:               " << metrics.longestLine        << std::endl;
	std::cout << "Average customer wait time: " << metrics.avgCustWait        << std::endl;
	std::cout << "Average teller wait time:   " << metrics.avgTellerWait      << std::endl;
	std::cout << "Average transaction time:   " << metrics.avgTransactionTime << std::endl;
	std::cout << "Max customer wait time:     " << metrics.maxCustWait        << std::endl;
	std::cout << "Max teller wait time:       " << metrics.maxTellerWait      << std::endl;
	std::cout << "Max transaction time:       " << metrics.maxTransactionTime << std::endl;

	return EXIT_SUCCESS;
}

