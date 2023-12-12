#include "./include/uart.h"
#include "./include/fifo.h" 
#include "./include/protocol.h"

#include <zephyr/drivers/uart.h>    /* for ADC API*/
#include <pthread.h>                /* for pthread_mutex_t */

#define UART_NODE DT_NODELABEL(uart0)   /* UART0 node ID*/

#define FATAL_ERR -1 /* Fatal error return code, app terminates */
#define OK 0         /* OK return code */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

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

            /* Check if the message fits in the fifo */
            if (evt->data.rx.len >= RXBUF_SIZE) {
                printk("FIFO is full \n\r");
                break;
            }

            /*
                RX timeout occurred and data is available:
                0. send reception confirmation
                1. cast message from uint8_t to char
                2. copy the data to the FIFO
                3. analyse the message (this will run in a separate thread)
                    3.1 if the message is a command, execute it (e.g. toggle led)
                    3.2 if the message is a request, send the response (e.g. send the value of a sensor)
            */

            /* Convert the message from uint8_t to char */
            char rx_chars[evt->data.rx.len];
            memcpy(
                rx_chars,
                &(rx_buf[evt->data.rx.offset]),
                evt->data.rx.len
            );

            /* Get reception confirmation */
            uint8_t rep_mesg[] = get_ack_msg(rx_chars);

            /* Send reception confirmation */
            err = uart_tx(uart_dev, rep_mesg, sizeof(rep_mesg), SYS_FOREVER_MS);
            if (err) {
                printk("uart_tx() error. Error code:%d\n\r",err);
            }

            /* Copy the received data to the FIFO */
            fifo_push(fifo, rx_chars, evt->data.rx.len);
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

int uart_send_data(uint8_t *data) {
    err = uart_tx(uart_dev, data, sizeof(data), SYS_FOREVER_MS);
    if (err) {
        printk("uart_tx() error. Error code:%d\n\r",err);
        return FATAL_ERR;
    }
    return OK;
}

struct FIFO *get_fifo(void) {
    return fifo;
}