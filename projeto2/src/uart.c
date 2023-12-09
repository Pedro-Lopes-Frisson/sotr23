#include "./include/uart.h"

#include <zephyr/drivers/uart.h>    /* for ADC API*/
#include <pthread.h>                /* for pthread_mutex_t */

#define UART_NODE DT_NODELABEL(uart0)   /* UART0 node ID*/

#define FATAL_ERR -1 /* Fatal error return code, app terminates */
#define OK 0         /* OK return code */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define RXBUF_SIZE 60                   /* RX buffer size */
#define TXBUF_SIZE 60                   /* TX buffer size */
#define RX_TIMEOUT 1000                 /* Inactivity period after the instant when last char was received that triggers an rx event (in us) */

/** \brief flag which warrants that the data transfer region is initialized
 * exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/* Struct for UART configuration (if using default values is not needed) */
const struct uart_config uart_cfg = {
    .baudrate = 115200,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
    .data_bits = UART_CFG_DATA_BITS_8,
    .flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};

/* UART related variables */
const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);
static uint8_t rx_buf[RXBUF_SIZE];      /* RX buffer, to store received data */
static uint8_t rx_chars[RXBUF_SIZE];    /* chars actually received  */
volatile int uart_rxbuf_nchar=0;        /* Number of chars currently on the rx buffer */

struct FIFO* fifo;

int uart_initialization(void) {
    /* Configure UART device */
    if (!uart_dev) {
        printk("UART device not found\n");
        return FATAL_ERR;
    }

    if (!device_is_ready(uart_dev)) {
        printk("UART device not ready\n");
        return FATAL_ERR;
    }

    if (uart_configure(uart_dev, &uart_cfg) != 0) {
        printk("Cannot configure UART device\n");
        return FATAL_ERR;
    }

    /* Configure UART rx timeout */
    if (uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), RX_TIMEOUT) != 0) {
        printk("Cannot configure UART rx\n");
        return FATAL_ERR;
    }

    /* Configure UART tx */
    if (uart_tx_enable(uart_dev, rx_buf, sizeof(rx_buf), SYS_FOREVER_MS) != 0) {
        printk("Cannot configure UART tx\n");
        return FATAL_ERR;
    }

    /* Configure UART rx callback */
    if (uart_callback_set(uart_dev, uart_rx_callback, NULL)) {
        printk("Cannot configure UART rx callback\n");
        return FATAL_ERR;
    }

    /* Initialize FIFO */
    fifo = fifo_init();
    if (fifo == NULL) {
        printk("Cannot initialize FIFO\n");
        return FATAL_ERR;
    }

    return OK;
}

void uart_rx_callback(const struct device *dev, struct uart_event *evt, void *user_data) {
    switch (evt->type) {

        case UART_TX_DONE:
            printk("UART_TX_DONE event \n\r");
            break;

        case UART_TX_ABORTED:
            printk("UART_TX_ABORTED event \n\r");
            break;

        case UART_RX_RDY:
            printk("UART_RX_RDY event \n\r");

            /* Check if the rx buffer is full */
            // if (uart_rxbuf_nchar >= RXBUF_SIZE) {
            //     printk("RX buffer is full \n\r");
            //     break;
            // }

            /* Check if the received data fits in the rx buffer */
            // if (evt->data.rx.len > (RXBUF_SIZE - uart_rxbuf_nchar)) {
            //     printk("RX no space available \n\r");
            //     break;
            // }

            /*
                RX timeout occurred and data is available:
                1. copy the data to the rx buffer
                2. understand the message
                    2.1 if the message is a command, execute it (e.g. toggle led)
                    2.2 if the message is a request, send the response (e.g. send the value of a sensor)
                3. send the response
                4. reset the rx buffer
            */

            /* Copy the received data to the rx buffer */
            memcpy(
                &rx_chars[uart_rxbuf_nchar],
                &(rx_buf[evt->data.rx.offset]),
                evt->data.rx.len
            ); 
            uart_rxbuf_nchar++;

            break;

        case UART_RX_BUF_REQUEST:
            printk("UART_RX_BUF_REQUEST event \n\r");
            break;

        case UART_RX_BUF_RELEASED:
            printk("UART_RX_BUF_RELEASED event \n\r");
            break;
        
        case UART_RX_DISABLED:
            printk("UART_RX_DISABLED event \n\r");

            /* Reset the rx buffer */
		    err =  uart_rx_enable(
                uart_dev ,
                rx_buf,
                sizeof(rx_buf),
                RX_TIMEOUT
            );
            if (err) {
                printk("uart_rx_enable() error. Error code:%d\n\r",err);
                int status = EXIT_FAILURE;
                pthread_exit(&status);
            }

            break;

        case UART_RX_STOPPED:
            printk("UART_RX_STOPPED event \n\r");
            break;

        default:
            printk("Unknown UART event type \n\r");
            break;
    }
}

void uart_send_data(uint8_t *data) {
    err = uart_tx(uart_dev, welcome_mesg, sizeof(welcome_mesg), SYS_FOREVER_MS);
    if (err) {
        printk("uart_tx() error. Error code:%d\n\r",err);
        return;
    }
}

struct FIFO *get_fifo(void) {
    return fifo;
}