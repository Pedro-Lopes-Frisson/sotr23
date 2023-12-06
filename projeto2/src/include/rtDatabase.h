
/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

static void initialization(void);

/*
 *  create a new CAB with specified id and with n_buffers and it's dimension
 * */
int openDb(const char *name);


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
int add_temp(double temp);


/*
 * destroy cab
 *
 * */
/*
 * destroy cab
 *
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
int add_temp(double temp) ;
int get_temps(double *t) ;
int reset_temps() ;
