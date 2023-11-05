/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 * 
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 * 
 * Responsible for displaying the variables in a window.
 */

/* Generic includes */
#include <fcntl.h>
#include <getopt.h> // For getting command args
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h> /* for semaphores */
#include <sys/mman.h> // For shmem and others
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Custom includes */
#include "../include/varsDisplayer.h"
#include "../include/object.h"

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

/* Shared memory and sempahore variables */
#define accessPerms                                                            \
  (S_IRUSR | S_IWUSR | S_IRGRP |                                               \
   S_IWGRP) /* Read and write perms for user and group */
#define SHMEMNAMESIZE 100
char varDispShMemName[SHMEMNAMESIZE]; /* name must start with "/" and contain up to 255
                                  chars withou further "/" */
int shMemSize = 0;             /* Size of shmem area, in bytes */
#define SEMNAMESIZE 100
char varDispSemName[SEMNAMESIZE]; /* just chars, please*/

/* Flags to signal that shmem/sem were indicated by the user */
char varDispShMemActiveFlag = 0; 
char varDispSemActiveFlag = 0;

void *varDispShMemPtr = NULL; /* Pointer top shared memory region */
sem_t *varDispSemAddr = NULL; /* Pointer to semaphore */
int fd = 0;                   /* File descriptor */

extern int n;

void help(char *progname) {
  printf("Usage: %s -v <shmem name> -d <sem name> -n <number of tasks>\n\r", progname);
  printf("Example: %s -v /varDispShMem -d /varDispSem -n 3\n\r", progname);
}

int main(int argc, char *argv[]){
  int opt, n;

  // if no arguments are passed, print help and exit
  if (argc == 1) {
    help(argv[0]);
    return -1;
  }

  while ((opt = getopt(argc, argv, "hv:d:")) != -1) {
    switch (opt) {
    case 'v':
      printf("optarg -v: %s\n", optarg);
      strcpy(varDispShMemName, optarg);
      varDispShMemActiveFlag = 1;
      break;
    case 'd':
      printf("optarg -d: %s\n", optarg);
      strcpy(varDispSemName, optarg);
      varDispSemActiveFlag = 1;
      break;
    case 'h':
    default:
      help(argv[0]);
      return -1;
    }

  fd = shm_open(varDispShMemName,    /* Open file */
                O_RDWR,             /* Open for read/write */
                accessPerms);       /* set access permissions */
  if (fd < 0)
  {
      printf("[shared memory reservation] Can't get file descriptor...\n\r");
  }

  /* Get the pointer */
  varDispShMemPtr = mmap(NULL,                   /* no hints on address */
                  sizeof(struct detected_obj) * n,              /* shmem size, in bytes */
                  PROT_READ | PROT_WRITE, /* allow read and write */
                  MAP_SHARED, /* modifications visible to other processes */
                  fd,         /* file descriptor */
                  0           /* no offset */
  );
  if (varDispShMemPtr == MAP_FAILED) {
    printf("[shared memory reservation] mmap failed... \n\r");
    return -1;
  }
  printf("Shared memory pointer obtained\n\r");

  /* Create semaphore */
  varDispSemAddr = sem_open(varDispSemName, /* semaphore name */
                            O_CREAT,        /* create the semaphore */
                            accessPerms,    /* protection perms */
                            0               /* initial value */
  );
  if (varDispSemAddr == SEM_FAILED) {
    printf("[semaphore creation] Error creating semaphore \n\r");
    return -1;
  }
  printf("Semaphore created\n\r");

  while (1) {
    if (!sem_wait(varDispSemAddr)) { /* enter monitor */
      // TODO: get data from shared memory

      printf("---------------------------\n\r");
      printf("%s at (%d,%d)\n", objs[0].obj_name, objs[0].cm_x, objs[0].cm_y);
      printf("---------------------------\n\r");
    }
  }

  return 0;
}

// ESTE CÓDIGO DEIXA DE FAZER SENTIDO
// TODO: apagar
// static void initialization(void) {
//   pthread_cond_init(&do_wait, NULL);
// }

// void open_rt_database(void){
//   if(n > MAX_TASKS){
//     perror("error on entering monitor(CF)");  /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }

//   pthread_once(&init, initialization);
// }

// void found_object(int cm_x, int cm_y){
//   if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
//     perror("error on entering monitor(CF)");  /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }

//   objs[0].cm_x = cm_x;
//   objs[0].cm_y = cm_y;
//   memcpy(objs[0].obj_name,OBSTACLE,OBSTACLE_S);

//   printf("---------------------------\n\r");
//   printf("%s at (%d,%d)\n", objs[0].obj_name, objs[0].cm_x, objs[0].cm_y);
//   printf("---------------------------\n\r");

//   if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
//     perror("error on exiting monitor(CF)");     /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }
// }
// void found_ball(int cm_x, int cm_y){
//   if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
//     perror("error on entering monitor(CF)");  /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }

//   objs[1].cm_x = cm_x;
//   objs[1].cm_y = cm_y;
//   memcpy (objs[1].obj_name,OBJECT_BALL, OBJECT_BALL_S);

//   printf("---------------------------\n\r");
//   printf("%s at (%d,%d)\n", objs[1].obj_name, objs[1].cm_x, objs[1].cm_y);
//   printf("---------------------------\n\r");

//   if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
//     perror("error on exiting monitor(CF)");     /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }

// }
// void found_landamark(int cm_x, int cm_y){
//   if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
//     perror("error on entering monitor(CF)");  /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }
//   objs[2].cm_x = cm_x;
//   objs[2].cm_y = cm_y;
//   memcpy (objs[2].obj_name,LANDMARK, LANDMARK_S);

//   printf("---------------------------\n\r");
//   printf("%s detected at (%d,%d)\n", objs[2].obj_name, objs[2].cm_x, objs[2].cm_y);
//   printf("---------------------------\n\r");

//   if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
//     perror("error on exiting monitor(CF)");     /* save error in errno */
//     int status = EXIT_FAILURE;
//     pthread_exit(&status);
//   }

// }
