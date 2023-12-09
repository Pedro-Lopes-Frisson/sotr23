#include "./include/protocol.h"
#include "./include/fifo.h"

#include <stdlib.h>

void analyse_msg(struct FIFO *fifo) {
    char *data = fifo_pop(fifo);
    if (data == NULL) {
        printk("Cannot pop data from FIFO\n");
        return;
    }

    // Analyse data
}