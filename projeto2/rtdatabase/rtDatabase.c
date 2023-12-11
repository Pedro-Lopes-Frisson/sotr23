#include "rtDatabase.h"
#include <zephyr/kernel.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/** \brief flag which warrants that the data transfer region is initialized
 * exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
K_MUTEX_DEFINE(accessCR);

/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

static int btns[4];
static int leds[4];
static double temps[20];
static int i;
static int idx;
static double min_temp;
static double max_temp;
static int temps_saved;

static void initialization(void) {
  // set initial values for each array
  for (i = 0; i < 4; i++) {
    leds[i] = 0;
    btns[i] = 0;
  }
  for (i = 0; i < 20; i++) {
    temps[i] = 0;
  }
  i = 0;
  idx = 0;
  min_temp = 0.0f;
  max_temp = 0.0f;
  temps_saved = 0;
}

int openDb(const char *name) {

	k_mutex_lock(&test_mutex, K_FOREVER);

  // initialize data structures
  initialization();

	k_mutex_unlock(&test_mutex);

  return EXIT_SUCCESS;
}

int set_btn(int btn, int val) {
	k_mutex_lock(&test_mutex, K_FOREVER);

  btns[btn] = val;

	k_mutex_unlock(&test_mutex);}

int toggle_btn(int btn) {
	k_mutex_lock(&test_mutex, K_FOREVER);

  btns[btn] = !btns[btn];

	k_mutex_unlock(&test_mutex);}


int set_led(int led, int val) {
	k_mutex_lock(&test_mutex, K_FOREVER);

  leds[led] = val;

	k_mutex_unlock(&test_mutex);}

int set_btns(const int *b) {
  if ((pthread_mutex_lock(&accessCR)) != 0) {
    perror("error on entering monitor(CF)");
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  for (i = 0; i < 4; i++) {
    btns[i] = b[i];
  }

	k_mutex_unlock(&test_mutex);}
int set_leds(const int *l) {
  if ((pthread_mutex_lock(&accessCR)) != 0) {
    perror("error on entering monitor(CF)");
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  for (i = 0; i < 4; i++) {
    leds[i] = l[i];
  }

	k_mutex_unlock(&test_mutex);}

int get_led(int led) {
	k_mutex_lock(&test_mutex, K_FOREVER);
  int val = leds[led];
	k_mutex_unlock(&test_mutex);
  return val;
}

int get_btn(int btn) {
	k_mutex_lock(&test_mutex, K_FOREVER);
  int val = btns[btn];
	k_mutex_unlock(&test_mutex);
  return val;
}


void get_leds(int * l){
  	k_mutex_lock(&test_mutex, K_FOREVER);


  for(i = 0; i < 4; i++){
    l[i] = leds[i];
  }

	k_mutex_unlock(&test_mutex);}
void get_btns(int * b){
	k_mutex_lock(&test_mutex, K_FOREVER);

  for(i = 0; i < 4; i++){
    b[i] = btns[i];
  }

	k_mutex_unlock(&test_mutex);}


int add_temp(double temp) {
	k_mutex_lock(&test_mutex, K_FOREVER);
  temps[idx] = temp;
  idx = (idx + 1) % 20;

  for (i = 0; i < temps_saved; i++) {
    if (min_temp > temps[i]) {
      min_temp = temps[i];
    }
    if (max_temp < temps[i]) {
      max_temp = temps[i];
    }
  }

  if (temps_saved == 0) {
    min_temp = temp;
    max_temp = temp;
  }

  temps_saved++;

	k_mutex_unlock(&test_mutex);}
/*
 * return the number of valiid temperatures that were copied to the input
 * parameter temps
 */
int get_temps(double *t) {
	k_mutex_lock(&test_mutex, K_FOREVER);
  int temps_returned = temps_saved;
  for (i = 0; i < temps_returned; i++) {
    t[i] = temps[i];
  }
	k_mutex_unlock(&test_mutex);
  return temps_returned;
}

int reset_temps() {
	k_mutex_lock(&test_mutex, K_FOREVER);

  temps_saved = 0;
  min_temp = 0.0f;
  max_temp = 0.0f;
  idx = 0;

	k_mutex_unlock(&test_mutex);}

int closeDb(void) { return EXIT_SUCCESS; }
