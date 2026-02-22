#ifndef __UART_RX_H__
#define __UART_RX_H__

#include "hardware/pio.h"
#include "hardware/uart.h"
#include "uart_rx.pio.h"

class UART_Rx {
public:
    UART_Rx(int rx_pin, int baud) {
	// We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
	// so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
	bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, &pio, &sm, &offset, rx_pin, 1, true);
	hard_assert(success);

	uart_rx_program_init(pio, sm, offset, rx_pin, baud);
    }

    bool is_empty() {
	return uart_rx_is_empty(pio, sm);
    }

    char getc() {
	return uart_rx_program_getc(pio, sm);
    }

private:
    PIO pio;
    uint sm;
    uint offset;
};

#endif
