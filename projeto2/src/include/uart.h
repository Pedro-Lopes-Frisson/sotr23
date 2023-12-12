#ifndef UART_H
#define UART_H

#include "fifo.h"     // Include the header file for the FIFO

#include <stdint.h>   // Include the necessary header file for uint8_t and uint16_t types

#define RXBUF_SIZE 60   /* RX buffer size */
#define TXBUF_SIZE 60   /* TX buffer size */
#define RX_TIMEOUT 1000 /* Inactivity period after the instant when last char was received that triggers an rx event (in us) */

/**
 * @brief initialize uart
 * 
 */
int uart_initialization(void);

/**
 * @brief callback function for uart rx
 * 
 */
void uart_rx_callback(const struct device *dev, struct uart_event *evt, void *user_data);

/**
 * @brief send data through uart
 * 
 */
int uart_send_data(uint8_t *data);

/**
 * @brief get the FIFO
 * 
 */
struct FIFO *get_fifo(void);

#endif