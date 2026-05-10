#include <stdio.h>
#include "pi.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "irq-buffered-reader.h"
#include "uart.h"

class UART_Hardware_Rx : public UART_Rx, public IRQBufferedReader {
public:
    UART_Hardware_Rx(uart_inst_t *uart, const char *name) : IRQBufferedReader(name), uart(uart) {
    }

    bool is_empty() override {
	return IRQBufferedReader::is_empty();
    }

    unsigned char getc() override {
	return IRQBufferedReader::getc();
    }

    void read(void *buffer, size_t n) {
	unsigned char *cur = (unsigned char *) buffer;
	while (n--) {
	    *cur++ = getc();
	}
    }

protected:
    bool read_char_if_available(unsigned char *chr) override {
	if (! uart_is_readable(uart)) return false;
	*chr = uart_getc(uart);
	return true;
    }

private:
    uart_inst_t *uart;
};

static UART_Hardware_Rx *uart_rx[2];

static void irq_handler() {
    uart_get_hw(uart0)->icr = UART_UARTICR_RXIC_BITS;
    uart_get_hw(uart1)->icr = UART_UARTICR_RXIC_BITS;
    if (uart_rx[0]) uart_rx[0]->on_irq();
    if (uart_rx[1]) uart_rx[1]->on_irq();
}

class UART_Hardware_Tx : public UART_Tx {
public:
    UART_Hardware_Tx(uart_inst_t *uart) : uart(uart) {
    }

    void putc(unsigned char c) override {
	uart_putc_raw(uart, c);
    }

    void write(const void *data, size_t n) {
	uart_write_blocking(uart, (const uint8_t *) data, n);
    }

private:
    uart_inst_t *uart;
};

static inline uart_inst_t *pin_to_uart(int pin, int *id = NULL, int *irq = NULL, const char **name = NULL) {
    switch(pin) {
    case 0: case 1:
    case 12: case 13:
    case 16: case 17:
	if (id) *id = 0;
	if (irq) *irq = UART0_IRQ;
	if (name) *name = "uart0-reader";
	return uart0;
    case 4: case 5:
    case 8: case 9:
	if (id) *id = 1;
	if (irq) *irq = UART1_IRQ;
	if (name) *name = "uart1-reader";
	return uart1;
    }
    assert(0);
    return NULL;
}

UART_Rx *pico_new_uart_rx(int pin, int baud) {
    int uart_id;
    int uart_irq;
    const char *name;
    uart_inst_t *uart = pin_to_uart(pin, &uart_id, &uart_irq, &name);

    if (! uart_is_enabled(uart)) {
	uart_init(uart, baud);
	//uart_set_hw_flow(uart, true, true);
    }

    gpio_set_function(pin, UART_FUNCSEL_NUM(uart_id, pin));

    /* Flush out any garbage on the input */
    while (uart_is_readable_within_us(uart, 100)) uart_getc(uart);

    /* Create the object before enabling interrupts in case we
     * get into a race condition with data arriving and notifying
     * the object before it's completely ready.
     */
    uart_rx[uart_id] = new UART_Hardware_Rx(uart, name);

    /* Setup interrupts for the uart */
    irq_set_exclusive_handler(uart_irq, irq_handler);
    irq_set_enabled(uart_irq, true);
    uart_set_irq_enables(uart, true, false);

    return uart_rx[uart_id];
}

UART_Tx *pico_new_uart_tx(int pin, int baud) {
    uart_inst_t *uart = pin_to_uart(pin);
    if (! uart_is_enabled(uart)) {
	uart_init(uart, baud);
	//uart_set_hw_flow(uart, true, true);
    }

    gpio_set_function(pin, UART_FUNCSEL_NUM(uart_id, pin));

    return new UART_Hardware_Tx(uart);
}

