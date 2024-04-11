#include "hardware/gpio.h"
#include "hardware/uart.h"

#include "pico/stdio.h"
#include "stdio.h"

#include "crc.h"
#include "serial.h"

#define SERIAL_UART uart0

#define SERIAL_BAUDRATE 112500


void serial_init() {
   
    // init data uart
    uart_init(SERIAL_UART, SERIAL_BAUDRATE);
    uart_set_hw_flow(SERIAL_UART, false, false);
    uart_set_translate_crlf(SERIAL_UART, false);
    gpio_set_function(SERIAL_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(SERIAL_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(SERIAL_CTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(SERIAL_RTS_PIN, GPIO_FUNC_UART);
}

#define END 0300     /* indicates end of packet */
#define ESC 0333     /* indicates byte stuffing */
#define ESC_END 0334 /* ESC ESC_END means END data byte */
#define ESC_ESC 0335 /* ESC ESC_ESC means ESC data byte */


void serial_write_data(uint8_t c){
    uart_putc_raw(SERIAL_UART, c);
}
