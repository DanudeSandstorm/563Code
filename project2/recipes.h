#ifndef RECIPES_H
#define RECIPES_H

#include "commands.h"
// Test Recipes

char demo1[] = {
	CMD_MOV(0), // There must NOT be intervening instructions in this group to allow
	CMD_MOV(5), // verification of default time delay.
	CMD_MOV(0),
	CMD_MOV(3),
	CMD_LOOP_START(0), //Test the default loop behavior.
	CMD_MOV(1),
	CMD_MOV(4),
	CMD_END_LOOP(),
	CMD_MOV(0),
	CMD_MOV(2),
	CMD_WAIT(0),
	CMD_MOV(3), // Move to an adjacent position to verify
	CMD_WAIT(0),
	CMD_MOV(2),
	CMD_MOV(3), // Measure the timing precision of the 9.3 second delay with an external
	CMD_WAIT(31), // timer.
	CMD_WAIT(31),
	CMD_WAIT(31),
	CMD_MOV(4),
	CMD_RECIPE_END()
};

#endif