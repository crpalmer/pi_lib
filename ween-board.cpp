#include "wb.h"
#include "time-utils.h"

class ween_board_input_t : public input_t {
public:
    ween_board_input_t(unsigned id) { this->id = id; this->debounce = 0; }
    void set_pullup_up() override { wb_set_pull_up(id, WB_PULL_UP_UP); }
    void set_pullup_down() override { wb_set_pull_up(id, WB_PULL_UP_DOWN); }
    void clear_pullup() override { wb_set_pull_up(id, WB_PULL_UP_NONE); }
    void set_debounce(unsigned ms) override { debounce = ms; }

    unsigned get() override {
	struct timespec start;

	nano_gettime(&start);
	while (wb_get(id)) {
	    if (nano_elapsed_ms_now(&start) >= (int) debounce) return 1;
	}
	return 0;
    }

private:
    unsigned id;
    unsigned debounce;
};

class ween_board_output_t : public output_t {
public:
    ween_board_output_t(unsigned bank, unsigned id) {
	this->bank = bank;
	this->id = id;
    }

    void set(bool value) override { wb_set(bank, id, value); };

private:
    unsigned bank;
    unsigned id;
};


input_t *wb_get_input(unsigned id)
{
    return new ween_board_input_t(id);
}

output_t *wb_get_output(unsigned bank, unsigned id)
{
    return new ween_board_output_t(bank, id);
}

output_t *wb_get_output(unsigned pin)
{
    return wb_get_output(pin / 8 + 1, pin % 8 + 1);
}
