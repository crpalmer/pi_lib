#ifndef __WB_H__
#define __WB_H__

#include <string.h>
#include "gp-input.h"
#include "gp-output.h"
#include "io.h"

class WeenBoard {
public:
    WeenBoard(int version = 2) : WeenBoard(input_maps[version-1], output_maps[version-1]) { }
    WeenBoard(const int inputs[8], const int outputs[2][8]) {
	for (int i = 0; i < 8; i++) {
	    this->inputs[i] = inputs[i];
	    for (int j = 0; j < 2; j++) {
		this->outputs[j][i] = outputs[j][i];
	    }
	}
    }

    Input *get_input(int id) {
	assert(id > 0 && id <= 8);
	return new GPInput(inputs[id-1]);
    }

    GPOutput *get_output(int bank, int id) {
	assert(bank == 1 || bank == 2);
	assert(id > 0 && id <= 8);
	return new GPOutput(outputs[bank-1][id-1]);
    }
	
private:
    int inputs[8], outputs[2][8];

    const int input_maps[2][8] = { { 14, 15, 18, 23, 22, 27, 17, 4 },
                                   { 23, 18, 15, 14, 22, 27, 17, 4 } };
    const int output_maps[2][2][8] = { { { 24, 25, 8, 7, 12, 16, 20, 21 }, { 26, 19, 13, 6, 5, 11, 9, 10 } },
                                       { { 24, 25, 8, 7, 12, 16, 20, 21 }, { 26, 18, 13, 6, 5, 11, 9, 10 } } };
};

#endif
