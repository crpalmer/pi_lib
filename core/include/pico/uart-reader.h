#ifndef __UART_READER_H__
#define __UART_READER_H__

#include "reader.h"

class UartReader : public Reader {
public:
    UartReader(size_t buf_size = 256);
    const char *readline() override;

private:
    char  *buf;
    size_t a_buf;
};

#endif
