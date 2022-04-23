#include <stdlib.h>
#include <climits>
#include "ble.h"
#include "time-utils.h"
#include "util.h"

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
}

bool
BLE::is_connected()
{
    return gpio_get(BLE_MODE_PIN);
}

void
BLE::accept_connection()
{
    printf("Waiting for the mode pin.\n");
    while(! is_connected()) {}

    /* drain any input garbage by sending an empty command and purging the input */
    uart_puts(uart, BLE_CMD_PREFIX);
    uart_puts(uart, "\r\n");
    ms_sleep(100);

    while (uart_is_readable(uart)) uart_getc(uart);
}

const char *
BLE::send_cmd(const char *cmd, int timeout)
{
    uart_puts(uart, BLE_CMD_PREFIX);
    uart_puts(uart, cmd);
    uart_putc(uart, '\r');
    uart_putc(uart, '\n');

    return receive_response(timeout);
}

const char *
BLE::receive_response(int timeout)
{
    int len = 0;
    struct timespec start;

    nano_gettime(&start);
    while (timeout < 0 || nano_elapsed_ms_now(&start) < timeout) {
	if (timeout < 0 && ! uart_is_readable_within_us(uart, (timeout - nano_elapsed_ms_now(&start)) * 1000)) {
	    break;
	} else {
	    char c = uart_getc(uart);
	    if (c == '\n' || c == '\r') {
		if (len > 0) {
		    buffer[len] = 0;
		    return buffer;
		}
	    } else if (len < sizeof(buffer) - 1) {
		buffer[len++] = c;
	    }
	}
    }

    return "timeout";
}

bool
BLE::wait_for_okay(int timeout)
{
    do {
	const char *response = receive_response(timeout);
	if (strcmp(response, "OK") == 0) return true;
	if (strcmp(response, "timeout") == 0) return false;
    } while (1);
}

int
BLE::get_baud()
{
    int baud = -1;

    const char *result = send_cmd("QT");
    if (result[0] == 'Q' && result[1] == 'T' && result[2] == '+') {
	int index = atoi(&result[3]);
	if (index > 0 && index <= n_CMD_baud) baud = CMD_baud[index-1];
	wait_for_okay();
    }
    return baud;
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
		return NULL;
	    }
	    return result;
	}
    }
    return "undefined baud";
}

void
BLE::set_ble_name(const char *name)
{
    char cmd[10+strlen(name)];

    sprintf(cmd, "BM%s", name);
    send_cmd(cmd);
}

void
BLE::get_ble_name(char *name)
{
    const char *result = send_cmd("TM");
    if (result[0] == 'T' && result[1] == 'M' && result[2] == '+') {
	strcpy(name, &result[3]);
	wait_for_okay();
    } else {
	strcpy(name, result);
    }
}
