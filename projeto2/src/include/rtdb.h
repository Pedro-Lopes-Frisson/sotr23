#ifndef RTDATABASE_H
#define RTDATABASE_H

/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */
static void rtdb_initialization(void);

/*
 *  create a new CAB with specified id and with n_buffers and it's dimension
 * */
int open_rtdb(const char *name);

/*
 * get state of button identified by *btn* in this database
 */
int get_btn(int btn);

/*
 * get state of led identified by *led* in this database
 */
int get_led(int led);

/*
 * add new temperature value
*/
int add_temp(uint8_t temp);

/*
 * stop rtdb thread
 * */
int closeDb(void);



void get_leds(int * l);
void get_btns(int * b);
int set_btn(int btn, int val) ;
int toggle_btn(int btn) ;
int set_led(int led, int val) ;
int set_btns(const int *b) ;
int set_leds(const int *l) ;
int get_led(int led) ;
int get_btn(int btn) ;
void get_leds(int * l);
void get_btns(int * b);
int get_temps(uint8_t *t) ;
void get_last_temp(uint8_t *t) ;
void get_max_temp(uint8_t *t) ;
void get_min_temp(uint8_t *t) ;
int reset_temps() ;

#endif