#ifndef __UART_TX_H__
#define __UART_TX_H__

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"

class UART_Tx {
public:
    UART_Tx(int tx_pin, int baud = 115200) {
	// This will find a free pio and state machine for our program and load it for us
	// We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
	// so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
	bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_tx_program, &pio, &sm, &offset, tx_pin, 1, true);
	if (! success) {
	    printf("UART_Tx: failed to claim a pio for pin %d\n", tx_pin);
	    exit(0);
	}
	uart_tx_program_init(pio, sm, offset, tx_pin, baud);
    }

    ~UART_Tx() {
	// This will free resources and unload our program
	pio_remove_program_and_unclaim_sm(&uart_tx_program, pio, sm, offset);
    }

    void putc(unsigned char c) {
	pio_sm_put_blocking(pio, sm, (uint32_t) c);
    }

    void puts(const char *s) {
	this->write((uint8_t *) s, strlen(s));
	putc('\n');
    }

    void write(uint8_t *data, size_t n) {
	while (n--) putc(*data++);
    }

private:
    PIO pio;
    uint sm;
    uint offset;
};

#endif
