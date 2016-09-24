#ifndef TASK_H
#define TASK_H

#include <stddef.h>
#include "commands.h"

#define STATUS_PAUSED (0x1)
#define STATUS_END (0x2) // end of recipe
#define STATUS_ERR_LOOP (0x4) // Nested loop error
#define STATUS_ERR (0x8) // Generic error

typedef struct
{
	char *recipe; // The recipe
	char *ip; // "instruction pointer"
	char *loopStart; // Address of the last loop start
	char loopCount; // Number of times we have left to loop
	char waitCount; // Number of times to wait before continuing
	char status; // Bitfield of statuses (see above)

	char servoPosition;
	char servoId; // id of the servo for this task

} Task;

// Glorified constructor
Task createTask(char *recipe, char servoId) {
	Task task;

	task.recipe = task.ip = recipe;
	task.loopStart = NULL;
	task.loopCount = task.waitCount = task.status = task.servoPosition = 0;

	task.servoId = servoId;

	return task;
}
#endif