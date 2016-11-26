#ifndef commands_h
#define commands_h

#include <stdint.h>

#define ERROR_CODE (0xFF);

#define CODE_MOV        (0x1)
#define CODE_WAIT       (0x2)
#define CODE_LOOP_START (0x4)
#define CODE_END_LOOP   (0x5)
#define CODE_RECIPE_END (0x0)


uint8_t mov(uint8_t position);
uint8_t wait(uint8_t cycles);
uint8_t loop_start(uint8_t iterations);
uint8_t end_loop();
uint8_t recipe_end();


#endif

