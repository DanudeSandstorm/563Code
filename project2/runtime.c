#include "runtime.h"
#include <stddef.h>

void dbg(char *str) {
	writeString(str);
}

// Runs a single step of the task's recipe
void stepTask(Task *task) {
	// Check for delays
	if (task->waitCount) {
		task->waitCount--;
		return;
	}

	// Check for paused/done
	if (task->status & (STATUS_PAUSED | STATUS_END)) {
		return;
	}

	// Check for error states
	if (task->status & (STATUS_ERR | STATUS_ERR_LOOP)) {
		return;
	}

	char op, arg;

	// TODO: decode

	task->ip++; // Increment IP

	switch (op) {
		case OP_MOV:
			if (op < 0 || op > 5) {
				dbg("MOV value out of range");
				task->status |= STATUS_ERR;
				return;
			}

			setServo(task->servoId, arg);
			break;

		case OP_WAIT:
			if (op < 0 || op > 31) {
				dbg("WAIT value out of range");
				task->status |= STATUS_ERR;
				return;
			}

			task->waitCount = arg;
			break;

		case OP_LOOP_START:
			if (op < 0 || op > 31) {
				dbg("LOOP value out of range");
				task->status |= STATUS_ERR;
				return;
			}

			if (task->loopStart) {
				dbg("ERR: Nested Loop");
				task->status |= STATUS_ERR_LOOP;
				return;
			}

			task->loopCount = arg;
			task->loopStart = task->ip;
			break;

		case OP_END_LOOP:
			if (!task->loopStart) {
				dbg("WARN: End loop without start loop");
				return;
			}

			if (task->loopCount) {
				task->loopCount--;
				task->ip = task->loopStart;
			} else {
				task->loopStart = NULL;
			}

			break;

		case OP_RECIPE_END:
			task->status |= STATUS_END;
			break;
	}
}