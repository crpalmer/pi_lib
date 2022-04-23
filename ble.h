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
    const char *set_baud(int baud);

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
    const char *send_cmd(const char *cmd, int timeout = -1);
    const char *receive_response(int timeout = -1);
    bool wait_for_okay(int timeout = -1);

    void enable_ble();
    void disable_spp();

    bool readline_internal(int timeout = -1);

    void drain_garbage();

private:
    uart_inst_t *uart;
    char *buffer;
    int   n_buffer;
    int   a_buffer;
};

/**
 * @brief Bluetooth connection detection pin
 */
#define BLE_MODE_PIN 15

#define GPIO_DOWN       false
#define GPIO_UP         true
#define UART_IRQ_OFF    false
#define UART_IRQ_ON     true

void delay(uint32_t s);
void delay_ms(uint32_t ms);
uint8_t Cmd_Process(uint8_t *data);
void GPIO_Init(uint8_t Pin,uint8_t Mode,uint8_t State);
void UART_RX_IRQ();
void UART_Init(bool Uart_Irq);
void BLE_Init();
void UART_RX_CMD();
void UART_RX();
void UART_TX(uint8_t *ch);

#endif
