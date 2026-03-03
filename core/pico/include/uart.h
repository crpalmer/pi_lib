#ifndef __UART_H__
#define __UART_H__

class UART_Rx {
public:
    virtual bool is_empty() = 0;
    virtual bool read(void *buffer, size_t n) = 0;
    virtual char getc() = 0;
    virtual bool gets(char *buffer, size_t n) = 0;
};

class UART_Tx { 
public:
    virtual void putc(unsigned char c) = 0;
    virtual void puts(const char *s) = 0;
    virtual void write(const void *data, size_t n) = 0;
};

UART_Rx *pico_new_uart_rx(int pin, int baud = 115200);
UART_Tx *pico_new_uart_tx(int pin, int baud = 115200);

UART_Rx *pico_new_pio_uart_rx(int pin, int baud = 115200);
UART_Tx *pico_new_pio_uart_tx(int pin, int baud = 115200);

#endif
