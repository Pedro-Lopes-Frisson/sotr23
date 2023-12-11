#include <stdbool.h>

#define SYNC_SYMBOL '!'
#define END_SYMBOL '#'

#define PC '0'
#define UC '1'

int msg_is_valid(char *msg);
int calculate_checksum(char *msg, int msg_size);
bool payload_is_valid(char *msg, int msg_size);
void analyse_msg(char *msg);