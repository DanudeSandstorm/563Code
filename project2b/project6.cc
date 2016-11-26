#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <cctype>     // For isupper
#include <sys/neutrino.h>
#include <pthread.h>  // For threads and mutexes
#include <unistd.h>   // For usleep

#include "servocontroller.h"
#include "commands.h"

struct RecipeState {
	uint8_t currentInstr;
	uint8_t loopbackInstr;
	uint8_t loopCount;
	uint8_t waitCount;
	bool    paused;
};

/* Function prototypes */
void  processUserCommands(void);
void  executeRecipeCommand(uint8_t command, uint8_t servoId);
void* executeRecipes(void* ignore);

/* Constants */
const uint32_t MIN_CLOCK_RESOLUTION = 1000 * 10; // 10us
uint8_t RECIPE[20]; // Defined in main


/* Globals */
struct RecipeState recipe1;
struct RecipeState recipe2;
ServoController servoController;
bool running;

int main(int argc, char *argv[]) {

	// Get root permissions to access I/O ports
	int privityErr = ThreadCtl(_NTO_TCTL_IO, NULL);
	if (privityErr == -1) {
		std::cout << "Could not get root access for I/O" << std::endl;
		return EXIT_FAILURE;
	}

	// Assign constant values
	RECIPE[0]  = mov(  0  );
	RECIPE[1]  = mov(  5  );
	RECIPE[2]  = mov(  0  );
	RECIPE[3]  = mov(  3  );
	RECIPE[4]  = loop_start( 0 );
	RECIPE[5]  = mov(  1  );
	RECIPE[6]  = mov(  4  );
	RECIPE[7]  = end_loop( );
	RECIPE[8]  = mov(  0  );
	RECIPE[9]  = mov(  2  );
	RECIPE[10] = wait( (uint8_t) 0 ); // Need to cast to get the compiler to shut up about ambiguous values
	RECIPE[11] = mov(  3  );
	RECIPE[12] = wait( (uint8_t) 0 );
	RECIPE[13] = mov(  2  );
	RECIPE[14] = mov(  3  );
	RECIPE[15] = wait( 31 );
	RECIPE[16] = wait( 31 );
	RECIPE[17] = wait( 31 );
	RECIPE[18] = mov(  4  );
	RECIPE[19] = recipe_end( );

	// Set the resolution of our clock higher for more accurate distance measurements.
	struct _clockperiod newRes;
	newRes.fract = 0; // This is always 0
	newRes.nsec = MIN_CLOCK_RESOLUTION;
	ClockPeriod(CLOCK_REALTIME, &newRes, NULL, 0);
	
	// Start servo threads
	servoController.startServos();
	
	// Start recipe running thread
	running = true;
	recipe1.paused = true;
	recipe2.paused = true;
	pthread_t recipeRunner;
	pthread_create(&recipeRunner, NULL, executeRecipes, '\0');
	
	// Process user input until EOF is entered
	processUserCommands();
	
	// Clean up and exit
	running = false;
	servoController.stopServos();
	pthread_join(recipeRunner, NULL);
	
	return EXIT_SUCCESS;
}

void* executeRecipes(void* ignore) {
	while (running) {
		if (!recipe1.paused) {
			executeRecipeCommand(RECIPE[recipe1.currentInstr], 1);
		}
		
		if (!recipe2.paused) {
			executeRecipeCommand(RECIPE[recipe2.currentInstr], 2);
		}
	}
	return NULL; // To satisfy pthread signature
}

void processUserCommands(void) {
	char input;
	uint8_t servoId;
	RecipeState* recipe;

	do {
		// Upper case characters control servo 1, lower case control servo 2
		input = getchar();
		getchar(); // throw out the \n

		if (std::isupper(input)) {
			servoId = 1;
			recipe = &recipe1;
		} else {
			servoId = 2;
			recipe = &recipe2;
		}

		switch(input) {
			
		// Pause Recipe execution
		case 'P':
		case 'p':
			recipe->paused = true;
			break;

		// Continue Recipe execution
		case 'C':
		case 'c':
			recipe->paused = false;
			break;

		// Move 1 position to the right if possible 
		case 'R':
		case 'r':
			if (servoController.servoUp(servoId) == SERVO_ERROR) {
				std::cout << "Error: Invalid Servo Position" << std::endl;
			}
			break;

		// Move 1 position to the left if possible 
		case 'L':
		case 'l':
			if (servoController.servoDown(servoId) == SERVO_ERROR) {
				std::cout << "Error: Invalid Servo Position" << std::endl;
			}
			break;

		// No-op no new override entered for selected servo 
		case 'N':
		case 'n':
			break;

		// Restart the recipe
		case 'B':
		case 'b':
			recipe->currentInstr = 0;
			break;

		default:
			std::cout << "Invalid input " << input << std::endl;
			break;
		}
	} while (input != EOF);

}

void executeRecipeCommand(uint8_t command, uint8_t servoId) {
	// Decode command
	uint8_t opcode = command >> 5;
	uint8_t value  = command & 0x1F;
	
	// Select appropriate recipe state based on servo ID
	RecipeState* recipeState;
	if (servoId == 1) {
		recipeState = &recipe1;
	} else if (servoId == 2) {
		recipeState = &recipe2;
	} else {
		std::cout << "Invalid servo ID" << std::endl;
	}
	
	switch (opcode) {
	
	// Move servo to position
	case CODE_MOV:
		if(servoController.setServoPosition(servoId, value) == SERVO_ERROR) {
			std::cout << "Error: Invalid Servo Position" << std::endl;
		} else {
			recipeState->currentInstr++;
		}
		break;
	
	// Wait n * 100 milliseconds
	case CODE_WAIT:
		recipeState->waitCount = value + 1; // Add 1 to value to wait(0) still waits
		while (recipeState->waitCount != 0) {
			usleep(1e5);
		}
		break;
	
	// Loop the following block instructions n times
	case CODE_LOOP_START:
		// Nested loops are not allowed
		if (recipeState->loopCount != 0) {
			std::cout << "Error: nested loops not allowed" << std::endl;
		}
		
		recipeState->loopCount = value;
		recipeState->loopbackInstr = recipeState->currentInstr + 1; // Goto the following instruction for each loop
		recipeState->currentInstr++;
		break;
		
	// Marks the end of an instruction loop block
	case CODE_END_LOOP:
		if (recipeState->loopCount != 0) {
			// Go to top of loop
			recipeState->loopCount--;
			recipeState->currentInstr = recipeState->loopbackInstr;
		}
		else {
			recipeState->currentInstr++;
		}
		break;
		
	// Marks the end of the recipe
	case CODE_RECIPE_END:
		break;
		
	default:
		std::cout << "Invalid command " << opcode << std::endl;
		break;
	}

	usleep(1e6); // Wait a second between commands
}
