#include "stm32l476xx.h"
#include "UART.h"
#include "GPIO.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void interpretCommands(char input) {
	switch (input) {
		// Pause recipe execution
		case 'P':
		case 'p':
			// TODO
			break;
		
		// Continue recipe execution
		case 'C':
		case 'c':
			// TODO
			break;
		
		// Move 1 position to right, if possible
		case 'R':
		case 'r':
			// TODO
			break;
		
		// Move 1 position to left, if possible
		case 'L':
		case 'l':
			// TODO
			break;
		
		// No-op
		case 'N':
		case 'n':
			// TODO
			break;
		
		// Restart recipe
		case 'B':
		case 'b':
			// TODO
			break;
	}
}


int main(void) {


	return 0;
}