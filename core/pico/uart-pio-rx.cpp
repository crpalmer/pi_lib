#include "pi.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "uart_rx.pio.h"
#include "uart.h"

class UART_PIO_Rx : public UART_Rx {
public:
    UART_PIO_Rx(int rx_pin, int baud) {
	// We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
	// so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
	bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, &pio, &sm, &offset, rx_pin, 1, true);
	hard_assert(success);

	uart_rx_program_init(pio, sm, offset, rx_pin, baud);
    }

    bool is_empty() override {
	return uart_rx_is_empty(pio, sm);
    }

    char getc() override {
	return uart_rx_program_getc(pio, sm);
    }

    void read(void *buffer, size_t n) {
	uint8_t *bytes = (uint8_t *) buffer;
	for (size_t i = 0; i < n; i++) {
	    bytes[i] = getc();
	}
    }

private:
    PIO pio;
    uint sm;
    uint offset;
};

UART_Rx *pico_new_pio_uart_rx(int pin, int baud) {
    return new UART_PIO_Rx(pin, baud);
}

