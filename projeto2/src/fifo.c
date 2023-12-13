#include "./include/fifo.h"
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct FIFO *fifo_init() {
  struct FIFO *fifo = (struct FIFO *)k_malloc(sizeof(struct FIFO));
  if (fifo == NULL) {
    printk("Cannot allocate memory for FIFO\n");
    return NULL;
  }

  fifo->head = NULL;
  fifo->tail = NULL;
  return fifo;
}

bool _is_empty(struct FIFO *fifo) { return (fifo->head == NULL); }

void fifo_push(struct FIFO *fifo, char *data) {
  struct NODE *new_node = (struct NODE *)k_malloc(sizeof(struct NODE));
  if (new_node == NULL) {
    printk("Cannot allocate memory for NODE\n");
    return;
  }

  // Copy data to new node
  for (int i = 0; i < MAX_CHARS; i++) {
    new_node->data[i] = data[i];
  }

  // Set next pointer to NULL because the new node is the last node
  new_node->next = NULL;

  if (_is_empty(fifo)) {
    // If FIFO is empty, set head and tail to new node
    // because the new node is the only node in FIFO
    fifo->head = new_node;
    fifo->tail = new_node;
  } else {
    // If FIFO is not empty, set tail->next to new node
    // and set tail to new node
    fifo->tail->next = new_node;
    fifo->tail = new_node;
  }
  printk("Wrote a new node\n");
}

int fifo_pop(struct FIFO *fifo, char * data) {
  if (_is_empty(fifo)) {
    printk("FIFO is empty\n");
    return 1;
  }
  printk("_is_empty() = %d\n", _is_empty(fifo));
  // Allocate memory for data
  if (data == NULL) {
    printk("Data is an invalid pointer\n");
    return 1;
  }

  // Copy data from head node to data
  for (int i = 0; i < MAX_CHARS; i++) {
    data[i] = fifo->head->data[i];
  }

  struct NODE *temp = fifo->head;
  if (fifo->head == fifo->tail) {
    // If FIFO has only one node, set head and tail to NULL
    fifo->head = NULL;
    fifo->tail = NULL;
  } else {
    // If FIFO has more than one node, set head to head->next
    fifo->head = fifo->head->next;
  }

  // k_free memory of old head node
  k_free(temp);

  return 0;
}
