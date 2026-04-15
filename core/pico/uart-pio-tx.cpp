#include "pi.h"
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"
#include "uart.h"

class UART_PIO_Tx : public UART_Tx {
public:
    UART_PIO_Tx(int tx_pin, int baud = 115200) {
        // This will find a free pio and state machine for our program and load it for us
        // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
        // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
        bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_tx_program, &pio, &sm, &offset, tx_pin, 1, true);
        if (! success) {
            printf("UART_PIO_Tx: failed to claim a pio for pin %d\n", tx_pin);
            exit(0);
        }
        uart_tx_program_init(pio, sm, offset, tx_pin, baud);
    }

    void putc(unsigned char c) override {
        pio_sm_put_blocking(pio, sm, (uint32_t) c);
    }

    void write(const void *datav, size_t n) override {
	uint8_t *data = (uint8_t *) datav;
        while (n--) putc(*data++);
    }

private:
    PIO pio;
    uint sm;
    uint offset;
};

UART_Tx *pico_new_pio_uart_tx(int pin, int baud) {
    return new UART_PIO_Tx(pin, baud);
}
