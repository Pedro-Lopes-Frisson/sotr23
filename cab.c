#include "cab.h"
#include <string.h>
void initialization(void) {}

bool open(const char *cab_name, const int max_buffers, const int dim) {
    max_buff = max_buffers;
    strncpy(cab_id, cab_name, 50);
    dim_buff = dim;
 }

bool close(void) { return false; }

void *putmes(struct CAB_BUFFER *c, unsigned char *data) { return nullptr; }

void *getmes(void) { return nullptr; }

void unget(struct CAB_BUFFER *buffer) {}
