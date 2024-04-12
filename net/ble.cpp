#include <stdlib.h>
#include <climits>
#include "ble.h"
#include "mem.h"
#include "time-utils.h"
#include "util.h"

#define BLE_MODE_PIN 15
#define UART_BAUD 115200

const char *BLE_CMD_PREFIX = "AT+";

const int CMD_baud[] = { 9600, 19200, 38400, 57600, 115200, 256000, 512000, 230400, 460800, 10000000, 31250, 2400, 4800 };
const int n_CMD_baud = sizeof(CMD_baud) / sizeof(CMD_baud[0]);

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

    a_buffer = 1024;
    buffer = (char *) fatal_malloc(a_buffer);
    n_buffer = 0;

    enable_ble();
    disable_spp();
    reset();
}

void
BLE::reset()
{
    send_cmd("CZ");		// reset the chip
    ms_sleep(1500);
    drain_garbage();
}

bool
BLE::is_connected()
{
    return gpio_get(BLE_MODE_PIN);
}

void
BLE::accept_connection()
{
    while(! is_connected()) { ms_sleep(100); }
    drain_garbage();
}

void
BLE::drain_garbage()
{
    ms_sleep(100);
    while (uart_is_readable_within_us(uart, 100*1000)) uart_getc(uart);
}

void
BLE::send_cmd(const char *cmd)
{
    uart_puts(uart, BLE_CMD_PREFIX);
    uart_puts(uart, cmd);
    uart_putc(uart, '\r');
    uart_putc(uart, '\n');
}

const char *
BLE::receive_response(int timeout)
{
    if (readline_internal(timeout, true)) return buffer;
    else return NULL;
}

bool
BLE::readline_internal(int timeout, bool ignore_connected_state)
{
    struct timespec start;

    nano_gettime(&start);
    while (ignore_connected_state || is_connected()) {
	int remaining = timeout - nano_elapsed_ms_now(&start);
	int ms;

	if (timeout <= 0 || remaining > 500) ms = 500;
	else if (remaining <= 0) break;
	else ms = remaining;

	if (uart_is_readable_within_us(uart, ms * 1000)) {
	    if (n_buffer >= a_buffer-1) {
		a_buffer *= 2;
		buffer = (char *) fatal_realloc(buffer, a_buffer);
	    }

	    char c = uart_getc(uart);
	    if (c == '\r') {
	    } else if (c == '\n') {
	        buffer[n_buffer] = 0;
		n_buffer = 0;
		return true;
	    } else {
		buffer[n_buffer++] = c;
	    }
	}
    }

    return false;
}

bool
BLE::readline(char *buf, int max_bytes, int timeout)
{
    if (readline_internal(timeout)) {
	strncpy(buf, buffer, max_bytes);
	return true;
    }
    return false;
}

int
BLE::get_baud()
{
    int baud = -1;

    send_cmd("QT");
    const char *result = receive_response();

    if (result[0] == 'Q' && result[1] == 'T' && result[2] == '+') {
	int index = atoi(&result[3]);
	if (index > 0 && index <= n_CMD_baud) baud = CMD_baud[index-1];
    }
    return baud;
}

void
BLE::set_baud(int baud)
{
    char cmd[10] = "CT+";

    for (int i = 0; i < n_CMD_baud; i++) {
	if (CMD_baud[i] == baud) {
	    sprintf(&cmd[3], "%02d", i);
	    send_cmd(cmd);
	    break;
	}
    }
}

void
BLE::set_ble_name(const char *name)
{
    char cmd[10+strlen(name)];

    sprintf(cmd, "BM%s", name);
    send_cmd(cmd);
    reset();
}

void
BLE::get_ble_name(char *name)
{
    send_cmd("TM");
    const char *result = receive_response();

    if (result[0] == 'T' && result[1] == 'M' && result[2] == '+') {
	strcpy(name, &result[3]);
    } else {
	strcpy(name, result);
    }
}

void
BLE::enable_ble()
{
    send_cmd("B401");
}

void
BLE::disable_spp()
{
    send_cmd("B500");
}
