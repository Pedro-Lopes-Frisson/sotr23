
/**
 * Test file for cabs
 *
 *
 *
 */

#include "include/cab.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h> // For semaphores
#include <signal.h>    // Timers
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h> // Timers
#include <unistd.h>

#define NS_IN_SEC 1000000000L
void write_to_cab(void);
void read_from_cab(void);
void read_from_cab_2(void);
void read_from_cab_4(void);
void callTasks(int frameCounter);
struct timespec TsAdd(struct timespec ts1, struct timespec ts2);
struct timespec TsSub(struct timespec ts1, struct timespec ts2);

unsigned char appName[] = "imageDisplayA"; /* Application name*/

pthread_t tIdWork[4];
int tid[4];
sem_t everyFrame;
sem_t every2Frame;
sem_t every4Frame;

/* Global variables */
struct IMAGE_DISPLAY *display = NULL; /* Image displayer struct */

/* Shared memory and sempahore variables */
#define accessPerms                                                            \
  (S_IRUSR | S_IWUSR | S_IRGRP |                                               \
   S_IWGRP) /* Read and write perms for user and group */
#define SHMEMNAMESIZE 100
char shMemName[SHMEMNAMESIZE]; /* name must start with "/" and contain up to 255
                                 chars withou further "/" */
char varDispShMemName[SHMEMNAMESIZE];

int shMemSize = 0; /* Size of shmem area, in bytes */
#define SEMNAMESIZE 100
char newDataSemName[SEMNAMESIZE]; /* just chars, please*/
char varDispSemName[SEMNAMESIZE]; /* just chars, please*/

char varDispShMemActiveFlag = 0;
char shMemActiveFlag =
    0; /* Flags to signal that shmem/sem were indicated by the user */
char newDataSemActiveFlag = 0;
char varDispSemActiveFlag = 0;

int fd = 0; /* File descriptor for shared memory */
int varDispFd = 0;

void *shMemPtr = NULL;        /* Pointer top shered memory region */
sem_t *newDataSemAddr = NULL; /* Pointer to semaphore */

void *varDispShMemPtr = NULL; /* Pointer top shered memory region */
sem_t *varDispSemAddr = NULL; /* Pointer to semaphore */

void main(int argc, char **argv) {

  int n = 4, width = 10, height = 1;

  openCab(appName, n + 1, width, height);
  printf("CAB created with %d buffers\n\r", n + 1);

  // initialize semaphore
  if (sem_init(&everyFrame, 0, 1) == -1) {
    perror("Semaphore initialization failed");
    exit(EXIT_FAILURE);
  }
  pthread_create(&tIdWork[1], NULL, (void *)read_from_cab, NULL);

  // initialize semaphore
  if (sem_init(&every2Frame, 0, 1) == -1) {
    perror("Semaphore initialization failed");
    exit(EXIT_FAILURE);
  }
  pthread_create(&tIdWork[2], NULL, (void *)read_from_cab_2, NULL);

  // initialize semaphore
  if (sem_init(&every4Frame, 0, 1) == -1) {
    perror("Semaphore initialization failed");
    exit(EXIT_FAILURE);
  }
  pthread_create(&tIdWork[3], NULL, (void *)read_from_cab_4, NULL);

  shMemSize = width * height;
  unsigned char *pixels = malloc(shMemSize);
  printf("Filesystem entry:       '/dev/shm%s'\n", shMemName);

  fd = shm_open("/webCM1",    /* Open file */
                O_RDWR,       /* Open for read/write */
                accessPerms); /* set access permissions */
  if (fd < 0) {
    printf("[shared memory reservation] Can't get file descriptor...\n\r");
    return -1;
  }
  printf("Shared memory file descriptor obtained\n\r");

  /* Get the pointer */
  shMemPtr = mmap(NULL,                   /* no hints on address */
                  shMemSize,              /* shmem size, in bytes */
                  PROT_READ | PROT_WRITE, /* allow read and write */
                  MAP_SHARED, /* modifications visible to other processes */
                  fd,         /* file descriptor */
                  0           /* no offset */
  );
  if (shMemPtr == MAP_FAILED) {
    printf("[shared memory reservation] mmap failed... \n\r");
    return -1;
  }
  printf("Shared memory pointer obtained\n\r");

  newDataSemAddr = sem_open("webCM",     /* semaphore name */
                            O_CREAT,     /* create the semaphore */
                            accessPerms, /* protection perms */
                            0            /* initial value */
  );
  if (newDataSemAddr == SEM_FAILED) {
    printf("[semaphore creation] Error creating semaphore \n\r");
    return -1;
  }
  printf("Semaphore created\n\r");

  // delay
  for (int j = 0; j < 1e9; j++) {
  }
  pthread_create(&tIdWork[0], NULL, (void *)write_to_cab, NULL);

  int s;
  for (int tnum = 0; tnum < 4; tnum++) {
    s = pthread_join(tIdWork[tnum], NULL);
    printf("Joined with thread %d; returned value was \n", tnum);
  }

  // finalize
  closeCab();
}

void write_to_cab() {
  int frameCounter = 0; // Activation counter
  int update;           // Flag to signal that min/max should be updated
  int periods = 201;
  unsigned char *data = malloc(sizeof(unsigned char) * 10);

  /* Set absolute activation time of first instance */

  /* Periodic jobs ...*/
  while (periods > 0) {
    // wait for new image
    if (!sem_wait(newDataSemAddr)) { /* sem_wait returns 0 on success */
      // reserve a buffer for writing
      struct CAB_BUFFER *cab = (struct CAB_BUFFER *)reserve();
      if (cab == NULL) {
        continue;
      }
      data[0] = (unsigned char)frameCounter; // save image to buffer
      data[1] = (unsigned char)frameCounter; // save image to buffer
      putmes(cab, data, 2);
      unget(cab);

      // check which tasks should be executed
      callTasks(frameCounter);
      // increment frame counter
      frameCounter++;
      printf("%d FrameCounter\n", frameCounter);
    } else {
      printf("[imageViewer] Error on sem_wait\n\r");
    }
    periods--;
  }

  return;
}

void callTasks(int frameCounter) {
  if (frameCounter % 1 == 0) {
    if ((sem_post(&everyFrame)) != 0) { /* enter monitor */
      perror("Error posting semapore for obstacle detection"); /* save error in
                                                                  errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }
  }
  if (frameCounter % 2 == 0) {
    if ((sem_post(&every2Frame)) != 0) { /* enter monitor */
      perror("Error posting semapore for obstacle detection"); /* save error in
                                                                  errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }
  }
  if (frameCounter % 4 == 0) {
    if ((sem_post(&every4Frame)) != 0) { /* enter monitor */
      perror("Error posting semapore for obstacle detection"); /* save error in
                                                                  errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }
  }
}

void read_from_cab(void) {
  struct CAB_BUFFER *c = NULL;
  int i = 0;
  int ct = 0;

  while (1) {
    if ((sem_wait(&everyFrame)) != 0) { /* enter monitor */
      perror("Error posting semapore for Landmark detection"); /* save error in
                                                                  errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }

    c = getmes();
    int r = ((int)c->img[0]) - 200;
    if (r == 0) {
      unget(c);
      int status = 0;
      pthread_exit(&status);
    }

    printf("message");
    printf("%d %d", 1, c->img[i]);
    printf("\n");
    ct++;
    printf("Read message %d\n", ct);
    unget(c);
  }
}
void read_from_cab_2(void) {
  struct CAB_BUFFER *c = NULL;
  int i = 0;
  int ct = 0;

  while (1) {
    if ((sem_wait(&every2Frame)) != 0) { /* enter monitor */
      perror("Error posting semapore for Landmark detection"); /* save error in
                                                                  errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }

    c = getmes();
    int r = c->img[0] - 200;
    if (r == 0) {
      unget(c);
      int status = 0;
      pthread_exit(&status);
    }

    printf("message");
    printf("%d: %d", 2, c->img[i]);
    printf("\n");
    ct++;
    printf("Read message %d\n", ct);
    for (int j = 0; j < 1e9; j++) {
    }
    unget(c);
  }
}
void read_from_cab_4(void) {
  struct CAB_BUFFER *c = NULL;
  int i = 0;
  int ct = 0;
  while (1) {
    if ((sem_wait(&every4Frame)) != 0) { /* enter monitor */
      perror("Error posting semapore for Landmark detection"); /* save error in
                                                                  errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }

    c = getmes();
    int r = c->img[0] - 200;
    if (r == 0) {
      unget(c);
      int status = 0;
      pthread_exit(&status);
    }

    printf("message");
    printf("%d: %d", 4, c->img[i]);
    printf("\n");
    ct++;
    printf("Read message %d\n", ct);

    for (int j = 0; j < 1e12; j++) {
    }
    unget(c);
  }
}

// Adds two timespect variables
struct timespec TsAdd(struct timespec ts1, struct timespec ts2) {

  struct timespec tr;

  // Add the two timespec variables
  tr.tv_sec = ts1.tv_sec + ts2.tv_sec;
  tr.tv_nsec = ts1.tv_nsec + ts2.tv_nsec;
  // Check for nsec overflow
  if (tr.tv_nsec >= NS_IN_SEC) {
    tr.tv_sec++;
    tr.tv_nsec = tr.tv_nsec - NS_IN_SEC;
  }

  return (tr);
}

// Subtracts two timespect variables
struct timespec TsSub(struct timespec ts1, struct timespec ts2) {
  struct timespec tr;

  // Subtract second arg from first one
  if ((ts1.tv_sec < ts2.tv_sec) ||
      ((ts1.tv_sec == ts2.tv_sec) && (ts1.tv_nsec <= ts2.tv_nsec))) {
    // Result would be negative. Return 0
    tr.tv_sec = tr.tv_nsec = 0;
  } else {
    // If T1 > T2, proceed
    tr.tv_sec = ts1.tv_sec - ts2.tv_sec;
    if (ts1.tv_nsec < ts2.tv_nsec) {
      tr.tv_nsec = ts1.tv_nsec + NS_IN_SEC - ts2.tv_nsec;
      tr.tv_sec--;
    } else {
      tr.tv_nsec = ts1.tv_nsec - ts2.tv_nsec;
    }
  }

  return (tr);
}
