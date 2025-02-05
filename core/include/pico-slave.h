#ifndef __PICO_SLAVE_H__
#define __PICO_SLAVE_H__

class PicoSlave {
public:
    PicoSlave();
    void writeline(const char *l);
    bool readline(char *buf, int len);

private:
    bool ensure_tty(void);
    int tty;
};

#endif
