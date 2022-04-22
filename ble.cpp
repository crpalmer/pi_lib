/*****************************************************************************
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of theex Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/

#include <stdlib.h>
#include <climits>
#include "ble.h"
#include "ble-cmd.h"
#include "time-utils.h"
#include "util.h"

#define UART_BAUD 115200

BLE::BLE(uart_inst_t *uart, int uart_tx_pin, int uart_rx_pin)
{
    this->uart = uart;

    gpio_init(BLE_MODE_PIN);
    gpio_set_dir(BLE_MODE_PIN, GPIO_IN);
    gpio_pull_down(BLE_MODE_PIN);

    uart_init(this->uart, UART_BAUD);
    gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(this->uart, false, false);
    uart_set_format(this->uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(this->uart, true);

    this->baud = -1;
}

void
BLE::accept_connection()
{
    printf("Waiting for the mode pin.\n");
    while(! gpio_get(BLE_MODE_PIN)) {}

    /* drain any input garbage by sending an empty command and purging the input */
    uart_puts(uart, BLE_CMD_PREFIX);
    uart_puts(uart, "\r\n");
    ms_sleep(100);

    while (uart_is_readable(uart)) uart_getc(uart);

    this->baud = get_baud();

    printf("Baud rate is: %d\n", this->baud);
}

const char *
BLE::send_cmd(const char *cmd, int timeout)
{
    int len = 0;
    struct timespec start;

    uart_puts(uart, BLE_CMD_PREFIX);
    uart_puts(uart, cmd);
    uart_putc(uart, '\r');
    uart_putc(uart, '\n');
    nano_gettime(&start);

    while (timeout < 0 || nano_elapsed_ms_now(&start) < timeout) {
	if (timeout < 0 && ! uart_is_readable_within_us(uart, (timeout - nano_elapsed_ms_now(&start)) * 1000)) {
	    return "timeout";
	} else {
	    char c = uart_getc(uart);
	    if (c == '\n' || c == '\r') {
		if (len > 0) {
		    buffer[len] = 0;
printf("got: %s\n", buffer);
		    return buffer;
		}
printf("ignored empty line\n");
	    } else if (len < sizeof(buffer) - 1) {
		buffer[len++] = c;
	    }
	}
    }

    return "timeout";
}

int
BLE::get_baud()
{
    const char *result = send_cmd(Baud_Rate_Query);
    if (result[0] == 'Q' && result[1] == 'T' && result[2] == '+') {
	int index = atoi(&result[3]);
	if (index >= 0 && index < n_CMD_baud) return CMD_baud[index];
    }
    return -1;
}

const char *
BLE::set_baud(int baud)
{
    char cmd[10] = "CT+";

    for (int i = 0; i < n_CMD_baud; i++) {
	if (CMD_baud[i] == baud) {
	    sprintf(&cmd[3], "%02d", i);
	    const char *result = send_cmd(cmd);
	    if (strcmp(result, "OK") == 0) {
		this->baud = baud;
		return NULL;
	    }
	    return result;
	}
    }
    return "undefined baud";
}
