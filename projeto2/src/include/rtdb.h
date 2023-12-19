#ifndef RTDATABASE_H
#define RTDATABASE_H

#include <stdint.h>

/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */
void rtdb_initialization(void);

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
void add_temp(int temp);

/*
 * stop rtdb thread
 * */
int closeDb(void);



void get_leds(int * l);
void get_btns(int * b);
void set_btn(int btn, int val) ;
void toggle_btn(int btn) ;
void set_led(int led, int val) ;
void set_btns(const int *b) ;
void set_leds(const int *l) ;
int get_led(int led) ;
int get_btn(int btn) ;
void get_leds(int * l);
void get_btns(int * b);
int get_temps(int *t) ;
void get_last_temp(int *t) ;
void get_max_temp(int *t) ;
void get_min_temp(int *t) ;
void reset_temps() ;

#endif