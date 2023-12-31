/* This file has all the functions related to the protocol
 * All the messages are in the form of:
 * {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
 *
 * sync_symbol: 1 byte (!)
 * tx_device_id: 1 byte (0-PC, 1-Nordic)
 * command_id: 1 byte
 * payload: 0-5 bytes
 * checksum: 3 bytes
 * end_symbol: 1 byte (#)
 *
 */

#include "./include/protocol.h"
#include "./include/fifo.h"
#include "./include/rtdb.h"
#include "./include/uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#define RXBUF_SIZE 60 /* RX buffer size */

char valid_commands[14] = {'0', '1', '2', '3', '4', '5', '6',
                           '7', 'A', 'B', 'C', 'D', 'E', 'Z'};
int payload_sizes[14] = {2, 4, 0, 0, 0, 0, 0, 0, 4, 4, 3, 3, 3, 2};

// Check if the message is valid
int msg_is_valid(char *msg) {
  // Check if the command ID is valid
  bool valid_command = false;
  for (int i = 0; i < 14; i++) {
    if (msg[2] == valid_commands[i]) {
      valid_command = true;
      break;
    }
  }

  if (!valid_command) {
    return 2;
  }

  // Get the size of the message
  int msg_size = 0;
  for (msg_size = 0; msg[msg_size] != '\0'; msg_size++)
    ;

  // Check if the message is in the form of
  // {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
  // and if the payload is valid
  if (msg[0] != SYNC_SYMBOL || msg[msg_size - 1] != END_SYMBOL ||
      !payload_is_valid(msg, msg_size)) {
    return 4;
  }

  // Get the checksum from the message
  char checksum[3];
  checksum[0] = msg[msg_size - 4];
  checksum[1] = msg[msg_size - 3];
  checksum[2] = msg[msg_size - 2];

  // Get the message without the checksum
  char msg_without_checksum[msg_size - 4];
  for (int i = 0; i < msg_size - 4; i++) {
    msg_without_checksum[i] = msg[i];
  }

  // Check if the checksum is correct
  if (atoi(checksum) !=
      calculate_checksum(msg_without_checksum, msg_size - 4)) {
    return 3;
  }

  return 1;
}

// Calculate the checksum of the message (without the checksum)
int calculate_checksum(char *msg, int msg_size) {
  int checksum = 0;
  for (int i = 1; i < msg_size; i++) {
    checksum += msg[i];
  }
  return checksum % 1000; // only the 3 most significant digits are used
}

char *get_payload(char *msg) {
  int msg_size = 0;
  for (msg_size = 0; msg[msg_size] != '\0'; msg_size++)
    ;

  char *payload = (char *)k_malloc(msg_size - (3 + 4));
  for (int i = 3, j = 0; i < msg_size - 4; i++, j++) {
    payload[j] = msg[i];
  }

  return payload;
}

int get_ack_msg(char *msg, uint8_t *answer, char *valid_message) {
  // ACK message example: !1Z01236#
  // Format:
  // {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
  // Payload: {command_id_received}{error_code}
  // Size: 1+1+1+2+3+1 = 9
  int error_code = -1;
  int end_symbol_idx = -1, sync_symbol_idx = -1;
  for (int i = RXBUF_SIZE; i >= 0; i--) {
    if (msg[i] == END_SYMBOL) {
      end_symbol_idx = i;
    }
    if (msg[i] == SYNC_SYMBOL && end_symbol_idx >= 0) {
      sync_symbol_idx = i;
      break;
    }
  }
  if (end_symbol_idx == -1 || sync_symbol_idx == -1 ||
      (sync_symbol_idx > end_symbol_idx)) {
    error_code = 4; // invalid frame structure
  }

  int j = 0;
  for (int i = sync_symbol_idx; i <= end_symbol_idx; i++) {
    valid_message[j] = msg[i];
    j++;
  }
  valid_message[j] = '\0';
  /* Get the error code */
  if (error_code != 4) {
    error_code = msg_is_valid(valid_message);
  }

  char payload[3] = {msg[2], error_code + '0', '\0'};

  /* Get the checksum */
  int checksum = calculate_checksum(payload, 2);

  sprintf(answer, "%c%c%c%s%d%c", SYNC_SYMBOL, UC, 'Z', payload, checksum,
          END_SYMBOL);
  return error_code;
}

// Check if the payload is valid
bool payload_is_valid(char *msg, int msg_size) {
  // payload_size = msg_size - (3 + 4)
  // 3 bytes for the {sync_symbol}{tx_device_id}{command_id}
  // 4 bytes for the {checksum}{end_symbol}

  // Check if the payload has the right length
  char command_id = msg[2];
  int payload_size = 0;
  printk("msg_size:    %d\n", msg_size);
  for (int i = 0; i < 14; i++) {
    if (command_id == valid_commands[i]) {
      payload_size = payload_sizes[i];
      break;
    }
  }

  return (msg_size - (3 + 4) == payload_size);
}

void analyse_msg(char *msg) {
  // We know that the message is valid
  // Get the command ID
  char command_id = msg[2];
  printk("\nCommand: |%c|\n", msg[2]);
  // 0 - Set the value of one of the digital outputs (Leds)
  // 1 - Set the value of all the digital outputs (atomic operation)
  // 2 - Read the value of the digital inputs
  // 3 - Read the value of the digital outputs
  // 4 - Read the last value of temperature
  // 5 - Read the last 20 values of temperature
  // 6 - Read the max and min temperature
  // 7 - Reset the temperature history (including max and min)
  // A - Digital inputs values (answer to cmd ‘2’)
  // B - Digital output values (answer to cmd ‘3’)
  // C - Last temperature value (answer to cmd ‘4’)
  // D - Last 20 temperature values (answer to cmd ‘5’)
  // E - Maximum and minimum temperature (answer to cmd ‘6’)

  // Since we are using the Nordic as the transmitter, we only need to handle
  // the commands that are sent from the PC to the Nordic (0-7). The other
  // commands are handled by the PC.

  switch (command_id) {
  case '0':
    printk("set_digital_output(msg)\n");
    set_digital_output(msg);
    printk("set_digital_output(msg)\n");
    break;
  case '1':
    printk("set_all_digital_outputs(msg)\n");
    set_all_digital_outputs(msg);
    printk("set_all_digital_outputs(msg)\n");
    break;
  case '2':
    read_digital_inputs(); // empty payload
    printk("read_digital_inputs(msg)\n");
    break;
  case '3':
    read_digital_outputs(); // empty payload
    printk("read_digital_outputs(msg)\n");
    break;
  case '4':
    read_last_temperature(); // empty payload
    printk("read_last_temperature(msg)\n");
    break;
  case '5':
    read_all_last_temperatures(); // empty payload
    break;
  case '6':
    read_min_max_temp(); // empty payload
    break;
  case '7':
    reset_temps();
    break;
  }
}

void set_digital_output(char *msg) {
  // Get the payload from the message
  printk("msg: --- %s\n", msg);
  char *payload = get_payload(msg);

  // Payload: ‘1’...’4’ (Led #, 1 byte) + {‘0’,’1’} (On/Off, one byte)

  // Get the pin number
  int pin_number = payload[0] - 48 - 1;

  // Get the pin value
  int pin_value = payload[1] - 48;
  printk("Pin number %d, pin val %d\n", pin_number, pin_value);
  // Set the pin value
  // TODO: toggle the pin value

  // Change the led value in the rtdb
  set_led(pin_number, pin_value);
}

void set_all_digital_outputs(char *msg) {
  // Get the payload from the message
  printk("1\n");
  char *payload = get_payload(msg);
  // Payload: {‘0’,’1’}+{‘0’,’1’}+{‘0’,’1’}+{‘0’,’1’} (On/Off, one byte, Led 1
  // to Led 4, left to right)
  printk("122\n");
  int leds[4];
  // Set the pin values
  for (int i = 0; i < 4; i++) {
    printk("Payloda int %d\n", (unsigned int)payload[i]);
    leds[i] = payload[i] - 48 == 0 ? 0 : 1;
  }
  set_leds(&leds);
}

void read_digital_inputs() {
  // Payload: {‘0’,’1’}+{‘0’,’1’}+{‘0’,’1’}+{‘0’,’1’} (On/Off, one byte, But 1
  // to But 4, left to right) Example: ‘0001’ means that only button 4 is
  // pressed FullMessage: ‘!0A0001{checksum}#’

  // Get values of the digital inputs from the rtdb
  int digital_inputs[4];
  get_btns(&digital_inputs);

  // Create payload
  char payload[] = {(char)digital_inputs[0] + 48, (char)digital_inputs[1] + 48,
                    (char)digital_inputs[2] + 48, (char)digital_inputs[3] + 48,
                    '\0'};

  // Get the checksum
  int checksum = calculate_checksum(payload, 4);

  // Send the values of the digital inputs to the PC
  uint8_t rep_mesg[TXBUF_SIZE];
  // {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
  // size = 1 + 1 + 1 + 4 + 3 + 1 = 11
  sprintf(rep_mesg, "%c%c%c%s%d%c", SYNC_SYMBOL, UC, 'A', payload, checksum,
          END_SYMBOL);
  printf("%c%c%c%s%d%c\n", SYNC_SYMBOL, UC, 'A', payload, checksum, END_SYMBOL);
  if (uart_send_data(rep_mesg) != 0) {
    printk("Error sending message\n\r");
  }
}

void read_digital_outputs() {
  // Payload: {‘0’,’1’}+{‘0’,’1’}+{‘0’,’1’}+{‘0’,’1’} (On/Off, one byte, Led 1
  // to Led 4, left to right) Example: ‘0001’ means that only Led 4 is on
  // FullMessage: ‘!0B0001{checksum}#’

  // Get values of the digital outputs from the rtdb
  int digital_outputs[4];
  get_leds(digital_outputs);

  // Create payload
  char payload[] = {digital_outputs[0] + '0', digital_outputs[1] + '0',
                    digital_outputs[2] + '0', digital_outputs[3] + '0', '\0'};

  // Get the checksum
  int checksum = calculate_checksum(payload, 4);

  // Send the values of the digital outputs to the PC
  uint8_t rep_mesg[TXBUF_SIZE];
  // {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
  // size = 1 + 1 + 1 + 4 + 3 + 1 = 11
  sprintf(rep_mesg, "%c%c%c%s%d%c", SYNC_SYMBOL, UC, 'B', payload, checksum,
          END_SYMBOL);
  printk("| %c%c%c%s%d%c\n", SYNC_SYMBOL, UC, 'B', payload, checksum,
         END_SYMBOL);
  if (uart_send_data(rep_mesg) != 0) {
    printk("Error sending message\n\r");
  }
}

void send_temp_message(int temp, char command_id) {
  // Payload: {‘+’,’-’} + {‘0’...’9’} + {‘0’...’9’} (i.e. signal followed by
  // temperature, integer with two digits) Example: ‘+023’ means that the
  // temperature is +23 degrees FullMessage: ‘!0C+023{checksum}#’

  // Create payload
  char payload[4];

  // Get the signal
  if (temp >= 0) {
    payload[0] = '+';
  } else {
    payload[0] = '-';
  }

  // Get the temperature
  int temperature = (int)temp;
  payload[1] = (temperature / 10) + '0';
  payload[2] = (temperature % 10) + '0';
  payload[3] = '\0';
  // Get the checksum
  int checksum = calculate_checksum(payload, 3);

  // Send the last temperature to the PC
  uint8_t rep_mesg[TXBUF_SIZE];
  // {sync_symbol}{tx_device_id}{command_id}{payload}{checksum}{end_symbol}
  // size = 1 + 1 + 1 + 3 + 3 + 1 = 10
  sprintf(rep_mesg, "%c%c%c%s%d%c", SYNC_SYMBOL, UC, command_id, payload,
          checksum, END_SYMBOL);
  printk("Temperature: %c%c%c%s%d%c\n", SYNC_SYMBOL, UC, command_id, payload,
         checksum, END_SYMBOL);
  if (uart_send_data(rep_mesg) != 0) {
    printk("Error sending message\n\r");
  }
}

void read_last_temperature() {
  // Get the last temperature from the rtdb
  int last_temperature;
  get_last_temp(&last_temperature);

  send_temp_message(last_temperature, 'C');
}

void temp_to_string(int temperature, int offset, char *payload) {
  // Get the signal
  if (temperature >= 0) {
    payload[offset + 0] = '+';
  } else {
    payload[offset + 0] = '-';
  }

  // Get the temperature
  payload[offset + 1] = (temperature / 10) + '0';
  payload[offset + 2] = (temperature % 10) + '0';
}

void read_all_last_temperatures() {
  int last_temperatures[20];
  int n_temps = get_temps(last_temperatures);
  char *payload =
      (char *)k_malloc(3 * sizeof(char) * n_temps +
                       sizeof(char)); // allocate enough characters to represent
  // n_temps in 3 characters : {‘+’,’-’} +
  // {‘0’...’9’} + {‘0’...’9’} + 1 for '\0'
  if (payload == NULL) {
    return;
  }
  for (int i = 0; i < n_temps; i++) {
    temp_to_string(last_temperatures[i], i * 3, payload);
  }

  payload[3 * sizeof(char) * n_temps] = '\0'; // ends the string
  int checksum =
      calculate_checksum(payload, 3 * sizeof(char) * n_temps + sizeof(char));
  uint8_t rep_mesg[TXBUF_SIZE];

  sprintf(rep_mesg, "%c%c%c%s%d%c", SYNC_SYMBOL, UC, 'D', payload, checksum,
          END_SYMBOL);
  printk("Temperature: %c%c%c%s%d%c\n", SYNC_SYMBOL, UC, 'D', payload, checksum,
         END_SYMBOL);
  if (uart_send_data(rep_mesg) != 0) {
    printk("Error sending message\n\r");
  }
}

void read_min_max_temp() {
  uint8_t max_temperature;
  get_max_temp(&max_temperature);
  uint8_t min_temperature;
  get_min_temp(&min_temperature);
  char *payload =
      (char *)k_malloc(3 * sizeof(char) * 2 +
                       sizeof(char)); // allocate enough characters to represent
  // n_temps in 3 characters : {‘+’,’-’} +
  // {‘0’...’9’} + {‘0’...’9’} + 1 for '\0'
  if (payload == NULL) {
    return;
  }
  temp_to_string(max_temperature, 0 * 3, payload);
  temp_to_string(min_temperature, 1 * 3, payload);

  payload[3 * sizeof(char) * 2] = '\0'; // ends the string
  int checksum =
      calculate_checksum(payload, 3 * sizeof(char) * 2 + sizeof(char));
  uint8_t rep_mesg[TXBUF_SIZE];

  sprintf(rep_mesg, "%c%c%c%s%d%c", SYNC_SYMBOL, UC, 'E', payload, checksum,
          END_SYMBOL);
  printk("Temperature: %c%c%c%s%d%c\n", SYNC_SYMBOL, UC, 'E', payload, checksum,
         END_SYMBOL);
  if (uart_send_data(rep_mesg) != 0) {
    printk("Error sending message\n\r");
  }
}
