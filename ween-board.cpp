#include "ween-board.h"
#include "wb.h"

class ween_board_input_t : public input_t {
public:
    ween_board_input_t(unsigned id) { this->id = id; }
    unsigned get() override { return wb_get(id); }
    void set_pullup_up() override { wb_set_pull_up(id, WB_PULL_UP_UP); }
    void set_pullup_down() override { wb_set_pull_up(id, WB_PULL_UP_DOWN); }
    void clear_pullup() override { wb_set_pull_up(id, WB_PULL_UP_NONE); }

private:
    unsigned id;
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


input_t *ween_board_t::get_input(unsigned id)
{
    return new ween_board_input_t(id);
}

output_t *ween_board_t::get_output(unsigned bank, unsigned id)
{
    return new ween_board_output_t(bank, id);
}
