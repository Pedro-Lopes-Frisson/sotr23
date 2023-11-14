/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 *
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 *
 * Responsible for launching all the tasks from the first SOTR project, except
 * the webCamCapture and the varsDisplayer.
 *  - This main file is responsible for launching all the tasks, initializing
 * the cab, both shared memories (from the webCamCapture to the imageHelper and
 * from all the image analysis task to the varsDisplayer) and semaphores/mutexes
 * that may be needed.
 */

/* Generic includes */
#include <fcntl.h>
#include <getopt.h> // For getting command args
#include <pthread.h>
#include <semaphore.h> // For semaphores
#include <stdio.h>
#include <string.h>   // For memcpy
#include <sys/mman.h> // For shmem and others
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* SDL includes */
#include <SDL2/SDL.h>

/* Custom includes */
#include "include/cab.h"         // For cab struct
#include "include/imageViewer.h" // For sdl image viewer
#include "include/objDetector.h"
#include "include/landmarkDetector.h"
#include "include/detectRedSquare.h"
#include "include/object.h"

/* Global settings */
#define FALSE 0 /* The usual true and false */
#define TRUE (!0)
#define SUCCESS 0          /* Program terminates normally */
#define IMGBYTESPERPIXEL 4 /* Number of bytes per pixel. */

#define MAX_WIDTH 1980  /* Sets the max allowed image width */
#define MAX_HEIGHT 1024 /* Sets the max allowed image height */

/* Function prototypes */
int main(int argc, char **argv);
static void help(const char *procname);
static int initializeMain();
void callTasks(int frameCounter);

/* Global variables */
unsigned char appName[] = "imageDisplayA"; /* Application name*/
struct IMAGE_DISPLAY *display = NULL;      /* Image displayer struct */

/* Shared memory and sempahore variables */
#define accessPerms                                                            \
  (S_IRUSR | S_IWUSR | S_IRGRP |                                               \
   S_IWGRP) /* Read and write perms for user and group */
#define SHMEMNAMESIZE 100
char shMemName[SHMEMNAMESIZE]; /* name must start with "/" and contain up to 255
                                  chars withou further "/" */
char varDispShMemName[SHMEMNAMESIZE];

int shMemSize = 0;             /* Size of shmem area, in bytes */
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

pthread_t tIdWork[100];
int tid[100];

/* SDL vars */
SDL_Event event;
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *screen_texture;

/*
 *	 Globals
 * */
int width, height, n = -1; /* n is the number of tasks */
sem_t landmarkCR, displayImageCR, detectObstaclesCR, redCR;

/* **************************************************
 * help() function
 *****************************************************/
void help(const char *procname) {
  printf("Usage: %s [OPTIONS]\n", procname);
  printf("\t -h : print this help dialog\n");
  printf("\t -x : specify width in pixels of the images being captured by some "
         "device.\n");
  printf("\t -y : specify height in pixels of the images being captured by "
         "some device.\n");
  printf("\t -n : specify number of tasks that shall be created. This is also "
         "the number of buffers - 1 in the CAB implementation.\n");
  printf("\t \t [-m]: shared memory name, in the form /string with no spaces. "
         "Should start with \"/\" and have no further \"/\"\n");
  printf("\t \t [-s]: semaphore name, in the form string with no spaces. \n");
  printf("\t \t [-v]: shared memory name for the var display, in the form /string with no spaces. "
         "Should start with \"/\" and have no further \"/\"\n");
  printf("\t \t [-d]: semaphore name for the var display, in the form string with no spaces. \n");
}

/* **************************************************
 * initializeMain() function
 *****************************************************/
int initializeMain(){
  // create CAB structure
  openCab(appName, n + 1, width, height);
  printf("CAB created with %d buffers\n\r", n + 1);

  SDL_Init(SDL_INIT_VIDEO);
  window =
      SDL_CreateWindow(appName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  /* Limit the window size so that it cannot */
  /* be smaller than teh webcam image size */
  SDL_SetWindowMinimumSize(window, width, height);
  SDL_RenderSetLogicalSize(renderer, width, height);
  SDL_RenderSetIntegerScale(renderer, 1);
  screen_texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                        SDL_TEXTUREACCESS_STREAMING, width, height);
  
  /* -------------------TASKS------------------- */

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

  // Create image displayer task
  printf("Initializing thread to display image\n");

  struct sched_param parmDisplayImg;
  pthread_attr_getschedparam (&attr, &parmDisplayImg);
  parmDisplayImg.sched_priority = 90;
  pthread_attr_setschedparam(&attr, &parmDisplayImg);

  pthread_create(&tIdWork[0], &attr, (void *)display_image, NULL);
  // initialize semaphore
  if (sem_init(&displayImageCR, 0, 1) == -1) {
      perror("Semaphore initialization failed");
      exit(EXIT_FAILURE);
  }

  // Create landmark finder task
  printf("Initializing thread to find landMarks\n");

  struct sched_param parmLandmark;
  pthread_attr_getschedparam (&attr, &parmLandmark);
  parmLandmark.sched_priority = 92;
  pthread_attr_setschedparam(&attr, &parmLandmark);

  pthread_create(&tIdWork[1], &attr, (void *)detect_landmark, NULL);
  // initialize semaphore
  if (sem_init(&landmarkCR, 0, 1) == -1) {
      perror("Semaphore initialization failed");
      exit(EXIT_FAILURE);
  }

  // Create object detector task
  printf("Initializing thread to detect objects\n");

  struct sched_param parmDetectObstacles;
  pthread_attr_getschedparam (&attr, &parmDetectObstacles);
  parmDetectObstacles.sched_priority = 96;
  pthread_attr_setschedparam(&attr, &parmDetectObstacles);

  pthread_create(&tIdWork[2], &attr, (void *)detect_obstacles_spiral, NULL);
  // initialize semaphore
  if (sem_init(&displayImageCR, 0, 1) == -1) {
      perror("Semaphore initialization failed");
      exit(EXIT_FAILURE);
  }

  // Create red detector task
  printf("Initializing thread to detect red objects\n");

  struct sched_param parmRed;
  pthread_attr_getschedparam (&attr, &parmRed);
  parmRed.sched_priority = 94;
  pthread_attr_setschedparam(&attr, &parmRed);

  pthread_create(&tIdWork[3], &attr, (void *)detect_red_square, NULL);
  // initialize semaphore
  if (sem_init(&redCR, 0, 1) == -1) {
      perror("Semaphore initialization failed");
      exit(EXIT_FAILURE);
  }

  /* -------------------TASKS------------------- */

  /* -------------------SHARED MEMORY AND SEMAPHORES------------------- */
  // create shared memory
  if (shMemActiveFlag) {
    shMemSize = width * height * IMGBYTESPERPIXEL;
    unsigned char *pixels = malloc(shMemSize);
    printf("Filesystem entry:       '/dev/shm%s'\n", shMemName );

    fd = shm_open(shMemName,    /* Open file */
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
  } else {
    printf("This process cannot run without a video input\n\r");
    return -1;
  }

  /* Create semaphore */
  if (newDataSemActiveFlag) {
    newDataSemAddr = sem_open(newDataSemName, /* semaphore name */
                              O_CREAT,        /* create the semaphore */
                              accessPerms,    /* protection perms */
                              0               /* initial value */
    );
    if (newDataSemAddr == SEM_FAILED) {
      printf("[semaphore creation] Error creating semaphore \n\r");
      return -1;
    }
    printf("Semaphore created\n\r");
  }

  /* create shared memory for var display */
  if (varDispShMemActiveFlag) {
    varDispFd = shm_open(varDispShMemName,    /* Open file */
                  O_RDWR,       /* Open for read/write */
                  accessPerms); /* set access permissions */
    if (varDispFd < 0) {
      printf("[varDisp shared memory reservation] Can't get file descriptor...\n\r");
      return -1;
    }
    printf("Var Display shared memory file descriptor obtained\n\r");

    int size = sizeof(struct detected_obj) * n;

    /* Get the pointer */
    varDispShMemPtr = mmap(NULL,                   /* no hints on address */
                    size,              /* shmem size, in bytes */
                    PROT_READ | PROT_WRITE, /* allow read and write */
                    MAP_SHARED, /* modifications visible to other processes */
                    varDispFd,         /* file descriptor */
                    0           /* no offset */
    );
    if (varDispShMemPtr == MAP_FAILED) {
      printf("[varDisp shared memory reservation] mmap failed... \n\r");
      return -1;
    }
    printf("Shared memory pointer obtained\n\r");
  }

  /* Create semaphore */
  if (varDispSemActiveFlag) {  
    varDispSemAddr = sem_open(varDispSemName, /* semaphore name */
                              O_CREAT,        /* create the semaphore */
                              accessPerms,    /* protection perms */
                              0               /* initial value */
    );
    if (varDispSemAddr == SEM_FAILED) {
      printf("[varDisp semaphore creation] Error creating semaphore \n\r");
      return -1;
    }
    printf("Semaphore created\n\r");
  }
  /* -------------------SHARED MEMORY AND SEMAPHORES------------------- */

  return 0;
}

/* **************************************************
 * main() function
 *****************************************************/
int main(int argc, char *argv[]) {
  int opt;

  // if no arguments are passed, print help and exit
  if (argc == 1) {
    help(argv[0]);
    return -1;
  }

  while ((opt = getopt(argc, argv, "hx:y:n:m:s:v:d:")) != -1) {
    switch (opt) {
    case 'x':
      width = atoi(optarg);
      if (width < 0 || width > MAX_WIDTH) {
        printf("Invalid x value.\n");
        help(argv[0]);
        return -1;
      }
      break;
    case 'y':
      height = atoi(optarg);
      if (height < 0 || height > MAX_HEIGHT) {
        printf("Invalid y value.\n");
        help(argv[0]);
        return -1;
      }
      break;
    case 'n':
      n = atoi(optarg);
      if (n < 1) {
        printf("Invalid n value.\n");
        help(argv[0]);
        return -1;
      }
      break;
    case 'm':
      printf("optarg -m: %s\n", optarg);
      strcpy(shMemName, optarg);
      shMemActiveFlag = 1;
      break;
    case 's':
      printf("optarg -s: %s\n", optarg);
      strcpy(newDataSemName, optarg);
      newDataSemActiveFlag = 1;
      break;
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
  }
  printf("width: %d, height: %d, n: %d\n", width, height, n);

  int init = initializeMain();
  if (init == -1) {
    printf("Error initializing main\n\r");
    return -1;
  }

  // start copying images from the shared memory to the cab
  int frameCounter = 0;
  while (1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        exit(0);
    }

    // wait for new image
    if (!sem_wait(newDataSemAddr)) { /* sem_wait returns 0 on success */
      // reserve a buffer for writing
      struct CAB_BUFFER *cab = (struct CAB_BUFFER *)reserve();
      if (cab == NULL) {
        continue;
      }
      // save image to buffer
      putmes(cab, shMemPtr, shMemSize);
      unget(cab);

      // check which tasks should be executed
      callTasks(frameCounter);
      // increment frame counter
      frameCounter++;
    } else {
      printf("[imageViewer] Error on sem_wait\n\r");
    }
  }

  /* Release resources and terminate */
  printf("releasing resources and ending \n\r");
  sem_destroy(&landmarkCR);
  closeCab();

  return SUCCESS;
}

/* **************************************************
 * callTasks() function
 *****************************************************/
void callTasks(int frameCounter) {
  if (frameCounter % 5 == 0) {
    if ((sem_post(&detectObstaclesCR)) != 0) { /* enter monitor */
        perror("Error posting semapore for obstacle detection");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }

    if ((sem_post(&displayImageCR)) != 0 ) { /* enter monitor */
        perror("Error posting semapore for image display");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }
  }

  if (frameCounter % 15 == 0) {
    if ((sem_post(&landmarkCR)) != 0) { /* enter monitor */
        perror("Error posting semapore for Landmark detection");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }
	//printf("Released landmark thread\n");
  }

  if (frameCounter % 8 == 0) {
    if ((sem_post(&redCR)) != 0) { /* enter monitor */
        perror("Error posting semapore for Landmark detection");  /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
    }
	  printf("Released landmark thread\n");
  }
}
