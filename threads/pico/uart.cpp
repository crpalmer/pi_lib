#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "uart.h"

class UART_Hardware_Rx : public UART_Rx {
public:
    UART_Hardware_Rx(uart_inst_t *uart) : uart(uart) {
    }

    bool is_empty() override {
	return ! uart_is_readable(uart);
    }

    char getc() override {
	return uart_getc(uart);
    }

    void read(void *buffer, size_t n) {
	uart_read_blocking(uart, (uint8_t *) buffer, n);
    }

private:
    uart_inst_t *uart;
};

class UART_Hardware_Tx : public UART_Tx {
public:
    UART_Hardware_Tx(uart_inst_t *uart) : uart(uart) {
    }

    void putc(char c) override {
	uart_putc_raw(uart, c);
    }

    void write(const void *data, size_t n) {
	uart_write_blocking(uart, (const uint8_t *) data, n);
    }

private:
    uart_inst_t *uart;
};

static inline uart_inst_t *pin_to_uart(int pin) {
    switch(pin) {
    case 0: case 1:
    case 12: case 13:
    case 16: case 17:
	return uart0;
    case 4: case 5:
    case 8: case 9:
	return uart1;
    }
    assert(0);
    return uart0;
}

UART_Rx *pico_new_uart_rx(int pin, int baud) {
    gpio_set_function(pin, GPIO_FUNC_UART);

    uart_inst_t *uart = pin_to_uart(pin);
    if (! uart_is_enabled(uart)) {
	uart_init(uart, baud);
	//uart_set_hw_flow(uart, true, true);
    }

    while (uart_is_readable_within_us(uart, 100)) uart_getc(uart);

    return new UART_Hardware_Rx(uart);
}

UART_Tx *pico_new_uart_tx(int pin, int baud) {
    gpio_set_function(pin, GPIO_FUNC_UART);

    uart_inst_t *uart = pin_to_uart(pin);
    if (! uart_is_enabled(uart)) {
	uart_init(uart, baud);
	//uart_set_hw_flow(uart, true, true);
    }

    return new UART_Hardware_Tx(uart);
}

