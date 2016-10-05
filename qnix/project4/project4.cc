#include <cstdlib>
#include <iostream>
#include <queue>
#include <pthread.h>


bool running = true;
int customerId = 0;
queue<int> customerQueue;

void spawnNewCustomer(void){
	customerQueue.push(customerId);
	customerId++;
}

int dequeueCustomer(void) {
	if(!customerQueue.empty()){
		return customerQueue.pop();
	}
	return -1;
}

void customerCreator(void){
	while(running){
		//get random interval to wait for next customer
		//int interval = rand(30...6*60)
		//wait for next customer
		//sleep(interval)
		//create new customer
		//spawnNewCustomer();
	}
}

int main(int argc, char *argv[]) {

	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	return EXIT_SUCCESS;
}

