#ifndef _BLE_H_
#define _BLE_H_

#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

class BLE {
public:
    BLE(uart_inst_t *uart = uart0, int uart_tx_pin = 0, int uart_rx_pin = 1);

    uart_inst_t *get_uart() { return uart; }

    int get_baud();
    void set_baud(int baud);

    void accept_connection();
    bool is_connected();

    int getc() { return uart_getc(uart); }
    void putc(int c) { uart_putc(uart, c); }
    void puts(char *s) { uart_puts(uart, s); }

    void set_ble_name(const char *name);
    void get_ble_name(char *name_ret);

    /* read a newline terminated line in atmost timeout ms.
       if a partial line is received in that time, it is internally
       buffered until the next call to this function completes the line.
     */
    bool readline(char *buffer, int max_bytes, int timeout = -1);

private:
    void send_cmd(const char *cmd);
    const char *receive_response(int timeout = 1000);
    void reset();

    void enable_ble();
    void disable_spp();

    bool readline_internal(int timeout = -1, bool ignore_connected_state = false);

    void drain_garbage();

private:
    uart_inst_t *uart;
    char *buffer;
    int   n_buffer;
    int   a_buffer;
};

#endif
