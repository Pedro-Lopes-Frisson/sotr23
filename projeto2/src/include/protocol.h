#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define SYNC_SYMBOL '!'
#define END_SYMBOL '#'

#define PC '0'
#define UC '1'


/// @brief check if the message is valid
/// @param msg the message to be checked
/// @return - 1 if the message is valid
/// @return - 2 if the command ID is not valid
/// @return - 3 if the checksum is not valid 
/// @return - 4 if the message is not in the form of {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
int msg_is_valid(char *msg);


/// @brief calculate the checksum of the message (without the checksum)
/// @param msg - the message to be checked
/// @param msg_size - the size of the message
/// @return the 3 most significant digits of the checksum
int calculate_checksum(char *msg, int msg_size);

/// @brief - get the payload from the message
/// @param msg - the message
/// @return the payload
char* get_payload(char *msg);

/// @brief from the message received from the PC, get the ack message to be sent to the PC
/// @param msg - the message received from the PC
/// @return the message to be sent to the PC
int get_ack_msg(char *msg, uint8_t* answer, char* valid_message);

bool payload_is_valid(char *msg, int msg_size);

void analyse_msg(char *msg);

void set_digital_output(char* msg);

void set_all_digital_outputs(char* msg);

void read_digital_inputs();

void read_digital_outputs();

void send_temp_message(int temp, char command_id);

void read_last_temperature();

void read_all_last_temperatures();

void read_min_max_temp();

#endif