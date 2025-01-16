#ifndef __UART_WRITER_H__
#define __UART_WRITER_H__

#include "writer.h"

class UartWriter : public Writer {
public:
    UartWriter();
    int write_str(const char *s) override;
};

#endif
