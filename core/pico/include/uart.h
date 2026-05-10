#ifndef __UART_H__
#define __UART_H__

#include <string.h>

class UART_Rx {
public:
    virtual bool is_empty() = 0;
    virtual void read(void *buffer, size_t n) = 0;
    virtual unsigned char getc() = 0;
    virtual void gets(char *buffer, size_t n) {
	for (size_t i = 0; i < n-1; i++) {
	    buffer[i] = getc();
	    if (buffer[i] == '\n') {
		buffer[i] = '\0';
		return;
	    }
	}
	buffer[n-1] = '\0';
    }
};

class UART_Tx { 
public:
    virtual void putc(unsigned char c) = 0;
    virtual void write(const char *s) {
	write(s, strlen(s));
    }
    virtual void write(const void *data, size_t n) = 0;
};

UART_Rx *pico_new_uart_rx(int pin, int baud = 115200);
UART_Tx *pico_new_uart_tx(int pin, int baud = 115200);

UART_Rx *pico_new_pio_uart_rx(int pin, int baud = 115200);
UART_Tx *pico_new_pio_uart_tx(int pin, int baud = 115200);

#endif
