#include <stdint.h>
#include "commands.h"

uint8_t command(uint8_t opCode, uint8_t value) {
	return ((opCode << 5) | value);
}

uint8_t mov(uint8_t position) {
	if (position < 0 || position > 5) {
		return ERROR_CODE;
	}
 
	return command(CODE_MOV, position);
}

uint8_t wait(uint8_t cycles) {
	if (cycles < 0 || 31 < cycles) {
		return ERROR_CODE;
	}

	return command(CODE_WAIT, cycles);
}

uint8_t loop_start(uint8_t iterations) {
	if (iterations < 0 || 31 < iterations) {
		return ERROR_CODE;
	}

	return command(CODE_LOOP_START, iterations);
}

uint8_t end_loop() {
	return command(CODE_END_LOOP , 0);
}

uint8_t recipe_end() {
	return command(CODE_RECIPE_END, 0);
}

