#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>
#define MAX_CHARS 30

struct NODE {
    char data[MAX_CHARS];
    struct NODE* next;
};

struct FIFO {
    struct NODE* head;
    struct NODE* tail;
};

/**
 * @brief init a new FIFO
 * 
 * @return FIFO* 
 */
struct FIFO* fifo_init();

/**
 * @brief private function to check if FIFO is empty
 * 
 * @param fifo 
 * @return true 
 * @return false 
 */
bool _is_empty(struct FIFO* fifo);

/**
 * @brief enqueue data to FIFO
 * 
 * @param fifo 
 * @param data 
 */
void fifo_push(struct FIFO* fifo, char* data);

/**
 * @brief dequeue data from FIFO
 * 
 * @param fifo 
 * @return char* 
 */
char* fifo_pop(struct FIFO* fifo);

#endif
