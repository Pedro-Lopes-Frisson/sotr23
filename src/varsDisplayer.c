/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 * 
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 * 
 * Responsible for displaying the variables in a window.
 */

#include "../include/varsDisplayer.h"
#include <pthread.h>
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/**
 * \brief flag which warrants that the data transfer region is initialized
 * exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief workers condition to wait for work assignment */
static pthread_cond_t do_wait;


static struct detected_obj *objs;

void open_rt_database(void){
  pthread_once(&init, initialization);
}

void initialization(void) {
  pthread_cond_init(&do_wait, NULL);
}

void found_object(int cm_x, int cm_y){
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }


  if ((pthread_cond_signal(&do_wait)) != 0) {
    perror("Signal failed! wait_for_work"); /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

}
void found_ball(int cm_x, int cm_y);
void found_landamark(int cm_x, int cm_y);
