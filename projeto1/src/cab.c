#include "../include/cab.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

static int max_buff;           // numero maximo de buffers
static int dim_buff_x;         // dimensoes do buffer
static int dim_buff_y;         // dimensoes do buffer
static char cab_id[MAX_CHARS]; // nome do cab

static struct CAB_BUFFER *free_b; // free buffer for writing
static struct CAB_BUFFER *mrb;    // most recent buffer
static struct CAB_BUFFER *buffers;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/**
 * \brief flag which warrants that the data transfer region is initialized
 * exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief workers condition to wait for work assignment */
static pthread_cond_t wait_for_written_buffers;

/** \brief distributor uses this condition to wait for the workers to be ready
 * for processing */
static pthread_cond_t workers_ready;

static void initialization(void) {
  pthread_cond_init(&wait_for_written_buffers, NULL);
  pthread_cond_init(&workers_ready, NULL);
}

int openCab(const char *cab_name, const int max_buffers, const int dim_x, const int dim_y) {

  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  max_buff = max_buffers;        // set max_buffers
  strncpy(cab_id, cab_name, 50); // copy cab_name to cab_id
  dim_buff_x = dim_x;            // set dimensions x and y
  dim_buff_y = dim_y;

  buffers =
      (struct CAB_BUFFER *)malloc(max_buffers * sizeof(struct CAB_BUFFER));

  if (buffers == NULL) {
    fprintf(stderr, "Structure could not be allocated!");
    return EXIT_FAILURE;
  }
  printf("%p\n", &buffers);

  for (int i = 0; i < max_buffers; i++) {
    // allocate data buffer of each CAB_BUFFER
    printf("dim_x %d, dim_y %d\n", dim_x, dim_y);
    buffers[i].img = (unsigned char *)malloc(dim_x * dim_y * IMGBYTESPERPIXEL *
                                             sizeof(unsigned char));
    printf("%d\n", max_buffers);
    if (buffers[i].img == NULL) {
      fprintf(stderr, "Structure could not be allocated!");
      return EXIT_FAILURE;
    }
    printf("buffer[%d]%p \t img %p\n",i, &(buffers[i]), buffers[i].img);
    // set use to 0
    buffers[i].use = 0;
    // set next buffer this is cyclic
    buffers[i].next = (struct CAB_BUFFER *)&(buffers[(i + 1) % max_buffers]);
  }

  free_b = (struct CAB_BUFFER *)&(buffers[0]);

  pthread_once(&init, initialization);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  return EXIT_SUCCESS;
}

int closeCab(void) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  for (int i = 0; i < max_buff; i++) {
    free(buffers[i].img);
  }
  free(buffers);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  return EXIT_SUCCESS;
}

void putmes(struct CAB_BUFFER *c, unsigned char *data, const int size) {

  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  memcpy(c->img, data, size);
  mrb = c;
  if ((pthread_cond_signal(&wait_for_written_buffers)) != 0) {
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

struct CAB_BUFFER *getmes(void) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  printf("getMes\n");
  // wait for written buffers
  if ((pthread_cond_wait(&wait_for_written_buffers,&accessCR)) != 0) {
    perror("Signal failed! wait_for_written_buffers"); /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  printf("Get Mes : %d\n", mrb->use);
  mrb->use++;
  printf("Get Mes : %d\n", mrb->use);
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  return mrb;
}

void unget(struct CAB_BUFFER *buffer) {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  buffer->use--;
  if (buffer->use == 0 && buffer != mrb) {
    printf("Freeing buffer;\n\n");
  }
  
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

struct CAB_BUFFER *reserve(void) {
  printf("Reserving buffer\n\r");
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  int i = 0;
  int found = -1;
  for( i = 0 ; i < max_buff ; i++ ){
    if(buffers[i].use <= 0){
      buffers[i].use=1;
      found = 0;
      break;
    }
  }
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  if(i == max_buff || found == -1){
    printf("Failed to reserve buffer\n");
    return NULL;
  }

  printf("\n\nReserved buffer I: %d\n", i);
  if( found == 0 )
	  return &buffers[i];
  return NULL;
}
