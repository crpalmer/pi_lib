#ifndef __WEEN_BOARD_H__
#define __WEEN_BOARD_H__

#include "io.h"

class ween_board_t {
public:
     output_t *get_output(unsigned bank, unsigned id);
     input_t *get_input(unsigned id);
};

#endif
