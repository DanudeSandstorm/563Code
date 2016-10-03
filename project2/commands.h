#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

// Command header file

// Command opcodes

#define OP_RECIPE_END (0x0)
#define OP_MOV (0x1)
#define OP_WAIT (0x2)
#define OP_LOOP_START (0x4)
#define OP_END_LOOP (0x5)

// CMD builders
#define CMD(op, arg) (uint8_t)(((op & 0x7) << 5) | (arg & 0x1F))
#define CMD_MOV(arg) CMD(OP_MOV, arg)
#define CMD_WAIT(arg) CMD(OP_WAIT, arg)
#define CMD_LOOP_START(arg) CMD(OP_LOOP_START, arg)
#define CMD_END_LOOP() CMD(OP_END_LOOP, 0)
#define CMD_RECIPE_END() CMD(OP_RECIPE_END, 0)

// Command funcs


#endif
