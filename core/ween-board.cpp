#include "wb.h"
#include "time-utils.h"

class WeenBoardInput : public Input {
public:
    WeenBoardInput(unsigned id) : Input() { this->id = id; }
    void set_pullup_up() override { wb_set_pull_up(id, WB_PULL_UP_UP); }
    void set_pullup_down() override { wb_set_pull_up(id, WB_PULL_UP_DOWN); }
    void clear_pullup() override { wb_set_pull_up(id, WB_PULL_UP_NONE); }

    unsigned get_fast() override {
	return wb_get(id);
    }

private:
    unsigned id;
};

class WeenBoardOutput : public Output {
public:
    WeenBoardOutput(unsigned bank, unsigned id) {
	this->bank = bank;
	this->id = id;
    }

    void set(bool value) override { wb_set(bank, id, value); };

private:
    unsigned bank;
    unsigned id;
};


Input *wb_get_input(unsigned id)
{
    return new WeenBoardInput(id);
}

Output *wb_get_output(unsigned bank, unsigned id)
{
    return new WeenBoardOutput(bank, id);
}

Output *wb_get_output(unsigned pin)
{
    return wb_get_output(pin / 8 + 1, pin % 8 + 1);
}
