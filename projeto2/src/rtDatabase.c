#include "./include/rtDatabase.h"
#include <locale.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/** \brief flag which warrants that the data transfer region is initialized
 * exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

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
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  // initialize data structures
  pthread_once(&init, initialization);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  return EXIT_SUCCESS;
}

int set_btn(int btn, int val) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  btns[btn] = val;

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

int toggle_btn(int btn) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  btns[btn] = !btns[btn];

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}


int set_led(int led, int val) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  leds[led] = val;

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

int set_btns(const int *b) {
  if ((pthread_mutex_lock(&accessCR)) != 0) {
    perror("error on entering monitor(CF)");
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  for (i = 0; i < 4; i++) {
    btns[i] = b[i];
  }

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}
int set_leds(const int *l) {
  if ((pthread_mutex_lock(&accessCR)) != 0) {
    perror("error on entering monitor(CF)");
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  for (i = 0; i < 4; i++) {
    leds[i] = l[i];
  }

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

int get_led(int led) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  int val = leds[led];
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  return val;
}

int get_btn(int btn) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  int val = btns[btn];
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  return val;
}


void get_leds(int * l){
    if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }


  for(i = 0; i < 4; i++){
    l[i] = leds[i];
  }

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}
void get_btns(int * b){
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  for(i = 0; i < 4; i++){
    b[i] = btns[i];
  }

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}


int add_temp(double temp) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
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

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}
/*
 * return the number of valiid temperatures that were copied to the input
 * parameter temps
 */
int get_temps(double *t) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  int temps_returned = temps_saved;
  for (i = 0; i < temps_returned; i++) {
    t[i] = temps[i];
  }
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  return temps_returned;
}

int reset_temps() {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  temps_saved = 0;
  min_temp = 0.0f;
  max_temp = 0.0f;
  idx = 0;

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

int closeDb(void) { return EXIT_SUCCESS; }
