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

    void set_ble_name(const char *name);
    void get_ble_name(char *name_ret);

private:
    const char *send_cmd(const char *cmd, int timeout = -1);
    const char *receive_response(int timeout = -1);
    bool wait_for_okay(int timeout = -1);

    void enable_ble();
    void disable_spp();

private:
    uart_inst_t *uart;
    char buffer[1024];
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
