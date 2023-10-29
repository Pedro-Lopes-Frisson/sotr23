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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

#define MAX_TASKS 10
#define OBJECT_BALL "BALL DETECTED"
#define OBJECT_BALL_S 13
#define OBSTACLE "OBSTACLE DETECTED"
#define OBSTACLE_S 17
#define LANDMARK "LANDMARK DETECTED"
#define LANDMARK_S 17

static struct detected_obj objs[MAX_TASKS];

extern int n;

static void initialization(void) {
  pthread_cond_init(&do_wait, NULL);
}
void open_rt_database(void){
  if(n > MAX_TASKS){
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  pthread_once(&init, initialization);
}


void found_object(int cm_x, int cm_y){
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  objs[0].cm_x = cm_x;
  objs[0].cm_y = cm_y;
  memcpy(objs[0].obj_name,OBSTACLE,OBSTACLE_S);
  printf("%s at (%d,%d)\n", objs[0].obj_name, objs[0].cm_x, objs[0].cm_y);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}
void found_ball(int cm_x, int cm_y){
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  objs[1].cm_x = cm_x;
  objs[1].cm_y = cm_y;
  memcpy (objs[1].obj_name,OBJECT_BALL, OBJECT_BALL_S);
  printf("%s at (%d,%d)\n", objs[1].obj_name, objs[1].cm_x, objs[1].cm_y);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

}
void found_landamark(int cm_x, int cm_y){
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  objs[2].cm_x = cm_x;
  objs[2].cm_y = cm_y;
  memcpy (objs[2].obj_name,LANDMARK, LANDMARK_S);
  printf("%s at (%d,%d)\n", objs[2].obj_name, objs[2].cm_x, objs[2].cm_y);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

}
