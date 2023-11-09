
/**
 * Test file for cabs
 *
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "include/cab.h"
#include <signal.h> // Timers
#include <time.h> // Timers
#include <semaphore.h> // For semaphores
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#define NS_IN_SEC 1000000000L
void write_to_cab(void);
void read_from_cab(void);
void read_from_cab_2(void);
void read_from_cab_4(void);
void callTasks(int frameCounter);
struct  timespec TsAdd(struct  timespec  ts1, struct  timespec  ts2);
struct  timespec TsSub(struct  timespec  ts1, struct  timespec  ts2);

unsigned char appName[] = "imageDisplayA"; /* Application name*/


pthread_t tIdWork[4];
int tid[4];
sem_t everyFrame;
sem_t every2Frame;
sem_t every4Frame;


void main(int argc, char ** argv){

  int n = 4, width = 10, height = 1;

  openCab(appName, n + 1, width, height);
  printf("CAB created with %d buffers\n\r", n + 1);
  pthread_create(&tIdWork[0], NULL, (void *)write_to_cab, NULL);
  
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

  int s;
   for (int tnum = 0; tnum < 4; tnum++) {
       s = pthread_join(tIdWork[tnum], NULL);
       printf("Joined with thread %d; returned value was \n",
	       tnum);
   }

  // finalize 
  closeCab();
}

void write_to_cab(){
    /* Timespec variables to manage time */
	struct timespec ts, // thread next activation time (absolute)
			ta, 		// activation time of current thread activation (absolute)
			tiat, 		// thread inter-arrival time,
			ta_ant, 	// activation time of last instance (absolute),
			tp; 		// Thread period

	/* Other variables */
	uint64_t min_iat, max_iat; // Hold the minimum/maximum observed inter arrival time
	int frameCounter = 0; 	// Activation counter
	int update; 	// Flag to signal that min/max should be updated
  	int periods = 20;
	unsigned char *data = malloc(sizeof(unsigned char) * 10);
	printf("Ollaa\n");

	/* Set absolute activation time of first instance */
  //
	tp.tv_nsec = 33333333; // 0 ns
	tp.tv_sec = 0;  // 5 s
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts = TsAdd(ts,tp);

	/* Periodic jobs ...*/
	while(periods > 0) {
		/* Wait until next cycle */
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,&ts,NULL);
		clock_gettime(CLOCK_MONOTONIC, &ta);
		ts = TsAdd(ts,tp);

		// reserve a buffer for writing
		struct CAB_BUFFER *cab = (struct CAB_BUFFER *)reserve();
		if (cab == NULL) {
		  continue;
		}
		memset(data, 'A' + frameCounter, 10);
		// save image to buffer
		putmes(cab, data , 10);
		unget(cab);
		printf("Wrote message %d\n", frameCounter );


		// check which tasks should be executed
		callTasks(frameCounter);
		// increment frame counter
		frameCounter++;
    		periods--;
	}
	struct CAB_BUFFER *cab = (struct CAB_BUFFER *)reserve();
	data[0] = 'e';
	data[1] = 'n';
	data[2] = 'd';
	data[3] = '\0';
	// save image to buffer
	putmes(cab, data , 10);
	unget(cab);
	callTasks(frameCounter);

    	return ;
}


void callTasks(int frameCounter) {
  if (frameCounter % 1 == 0) {
    if ((sem_post(&everyFrame)) != 0) { /* enter monitor */
        perror("Error posting semapore for obstacle detection");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }
  }
  if (frameCounter % 2 == 0) {
    if ((sem_post(&every2Frame)) != 0) { /* enter monitor */
        perror("Error posting semapore for obstacle detection");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }
  }
  if (frameCounter % 4 == 0) {
    if ((sem_post(&every4Frame)) != 0) { /* enter monitor */
        perror("Error posting semapore for obstacle detection");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }
  }
}

void read_from_cab(void) {
	struct CAB_BUFFER *c = NULL;
	int i=0;
	int ct = 0;
	while(1){
	    if (c != NULL)
	    	unget(c);

		if ((sem_wait(&everyFrame)) != 0) { /* enter monitor */
		    perror("Error posting semapore for Landmark detection");  /* save error in errno */
		    int status = EXIT_FAILURE;
		    pthread_exit(&status);
		}
		c = getmes();
		int r = strncmp("end", c->img, 4 );
		if (r == 0) {
			int status = 0;
			printf("aaaaaaaaaaaaaaaaaaaaaaa%daaaaaaaaaaaaaaaaaaa\n\n\n\n\n", r);
			pthread_exit(&status);
		}
		printf("message:\n");
		for (i = 0; i < 10; i++){
			printf("%d", c->img[i]);
		}
		printf("\n");
		ct++;
		printf("Read message %d\n", ct);
	}
}
void read_from_cab_2(void){

	struct CAB_BUFFER *c = NULL;
	int i=0;
	int ct = 0;
	while(1){
	    if (c != NULL)
	    	unget(c);

		if ((sem_wait(&every2Frame)) != 0) { /* enter monitor */
		    perror("Error posting semapore for Landmark detection");  /* save error in errno */
		    int status = EXIT_FAILURE;
		    pthread_exit(&status);
		}
		c = getmes();
		int r = strncmp("end", c->img, 4 );
		if (r == 0) {
			int status = 0;
			pthread_exit(&status);
		}
		for (int j = 0 ; j < 1e5; j++){}
		printf("message:\n");
		for (i = 0; i < 10; i++){
			printf("%d", c->img[i]);
		}
		printf("\n");
		ct++;
		printf("Read message %d\n", ct);
	}
}
void read_from_cab_4(void){

	struct CAB_BUFFER *c = NULL;
	int i=0;
	int ct = 0;
	while(1){
	    if (c != NULL)
	    	unget(c);

		if ((sem_wait(&every4Frame)) != 0) { /* enter monitor */
		    perror("Error posting semapore for Landmark detection");  /* save error in errno */
		    int status = EXIT_FAILURE;
		    pthread_exit(&status);
		}
		c = getmes();
		int r = strncmp("end", c->img, 4 );
		if ( r == 0) {
			int status = 0;
			pthread_exit(&status);
		}
		for (int j = 0 ; j < 1e9; j++){}
		printf("message:\n");
		for (i = 0; i < 10; i++){
			printf("%d", c->img[i]);
		}
		printf("\n");
		ct++;
		printf("Read message %d\n", ct);
	}
}

// Adds two timespect variables
struct  timespec  TsAdd(struct  timespec  ts1, struct  timespec  ts2){

    struct  timespec  tr;

	// Add the two timespec variables
    	tr.tv_sec = ts1.tv_sec + ts2.tv_sec ;
    	tr.tv_nsec = ts1.tv_nsec + ts2.tv_nsec ;
	// Check for nsec overflow
	if (tr.tv_nsec >= NS_IN_SEC) {
        	tr.tv_sec++ ;
		tr.tv_nsec = tr.tv_nsec - NS_IN_SEC ;
    	}

    return (tr) ;
}

// Subtracts two timespect variables
struct  timespec  TsSub (struct  timespec  ts1, struct  timespec  ts2) {
  struct  timespec  tr;

  // Subtract second arg from first one
  if ((ts1.tv_sec < ts2.tv_sec) || ((ts1.tv_sec == ts2.tv_sec) && (ts1.tv_nsec <= ts2.tv_nsec))) {
    // Result would be negative. Return 0
    tr.tv_sec = tr.tv_nsec = 0 ;
  } else {
	// If T1 > T2, proceed
        tr.tv_sec = ts1.tv_sec - ts2.tv_sec ;
        if (ts1.tv_nsec < ts2.tv_nsec) {
            tr.tv_nsec = ts1.tv_nsec + NS_IN_SEC - ts2.tv_nsec ;
            tr.tv_sec-- ;
        } else {
            tr.tv_nsec = ts1.tv_nsec - ts2.tv_nsec ;
        }
    }

    return (tr) ;

}
