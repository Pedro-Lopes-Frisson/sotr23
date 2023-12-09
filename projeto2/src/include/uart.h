#include "./fifo.h"

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
void uart_send_data(uint8_t *data, uint16_t len);

struct FIFO *get_fifo(void);