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
#include <stdio.h>
#include <string.h> 	// For memcpy
#include <getopt.h> 	// For getting command args
#include <semaphore.h>	// For semaphores
#include <sys/mman.h>	// For shmem and others
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/* SDL includes */
#include <SDL2/SDL.h>
/* Custom includes */
#include "include/cab.h" // For cab struct
#include "include/imageViewer.h" // For sdl image viewer

/* Global settings */
#define FALSE   0   /* The usual true and false */
#define TRUE (!0)
#define SUCCESS 0           /* Program terminates normally */
#define IMGBYTESPERPIXEL 4 	/* Number of bytes per pixel. */

#define MAX_WIDTH	1980	/* Sets the max allowed image width */
#define MAX_HEIGHT	1024	/* Sets the max allowed image height */

/* Function prototypes */
int main(int argc, char **argv);
void help(const char *procname);
void callTasks(int frameCounter);
void display_image(void);

/* Global variables */
unsigned char appName[]="imageDisplay"; /* Application name*/
struct IMAGE_DISPLAY *display = NULL; /* Image displayer struct */

/* Shared memory and sempahore variables */
#define accessPerms (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) /* Read and write perms for user and group */
#define SHMEMNAMESIZE 100
char shMemName[SHMEMNAMESIZE]; /* name must start with "/" and contain up to 255 chars withou further "/" */
int shMemSize = 0; /* Size of shmem area, in bytes */
#define SEMNAMESIZE 100
char newDataSemName[SEMNAMESIZE]; /* just chars, please*/

char shMemActiveFlag = 0; /* Flags to signal that shmem/sem were indicated by the user */
char newDataSemActiveFlag = 0;

int fd=0; /* File descriptor for shared memory */

void * shMemPtr = NULL; 		/* Pointer top shered memory region */
sem_t * newDataSemAddr = NULL;	/* Pointer to semaphore */

pthread_t tIdWork[100];
int tid[100];

SDL_Event event;
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * screen_texture;

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
}
int width, height, n; /* n is the number of tasks */

/* **************************************************
 * main() function
*****************************************************/
int main(int argc, char* argv[]) {
    int opt;

    // if no arguments are passed, print help and exit
    if (argc == 1) {
        help(argv[0]);
        return -1;
    }

    while ((opt = getopt(argc, argv, "hx:y:n:m:s:")) != -1) {
        switch (opt) {
            case 'x':
                width = atoi(optarg);
                // TODO: change this to constants using #define min and max
                if (width < 640 || width > 5000) {
                    printf("Invalid x value.\n");
                    help(argv[0]);
                    return -1;
                }
                break;
            case 'y':
                height = atoi(optarg);
                // TODO: change this to constants using #define min and max
                if (height < 480 || height > 5000) {
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
            case 'h':
            default:
                help(argv[0]);
                return -1;
        }
    }
    printf("width: %d, height: %d, n: %d\n", width, height, n);

    // create CAB structure
    openCab(appName, n+1, width, height);
    printf("CAB created with %d buffers\n\r", n+1);

    // initialize image displayer
    //display = initDisplayer(height, width, IMGBYTESPERPIXEL, appName);
    printf("Image displayer initialized\n\r");
    
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(appName,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height,SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window,
                    -1, SDL_RENDERER_PRESENTVSYNC);

    /* Limit the window size so that it cannot */
    /* be smaller than teh webcam image size */
    SDL_SetWindowMinimumSize(window, width, height);

    SDL_RenderSetLogicalSize(renderer, width, height);
    SDL_RenderSetIntegerScale(renderer, 1);

    screen_texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
		width, height);
    renderer = SDL_CreateRenderer(window,
			-1, SDL_RENDERER_PRESENTVSYNC);

    shMemSize = width * height * IMGBYTESPERPIXEL;
    unsigned char * pixels = malloc(shMemSize);

    fd = shm_open(shMemName,    /* Open file */
			O_RDWR , 			/* Open for read/write */
			accessPerms);       /* set access permissions */
    if (fd < 0) {
		printf("[shared memory reservation] Can't get file descriptor...\n\r");
		return -1;
	}
    printf("Shared memory file descriptor obtained\n\r");

    /* Get the pointer */
	shMemPtr = mmap(NULL,       /* no hints on address */
		shMemSize,   			/* shmem size, in bytes */
		PROT_READ | PROT_WRITE, /* allow read and write */
		MAP_SHARED, /* modifications visible to other processes */
		fd,         /* file descriptor */
		0           /* no offset */
    );
    if( shMemPtr == MAP_FAILED){
		printf("[shared memory reservation] mmap failed... \n\r");
		return -1;
	}
    printf("Shared memory pointer obtained\n\r");

    /* Create semaphore */
	newDataSemAddr = sem_open(newDataSemName, 	/* semaphore name */
        O_CREAT,       					/* create the semaphore */
        accessPerms,   					/* protection perms */
        0            					/* initial value */
    );
	if (newDataSemAddr == SEM_FAILED) {
		printf("[semaphore creation] Error creating semaphore \n\r");
		return -1;
	}
    printf("Semaphore created\n\r");

    // start copying images from the shared memory to the cab
    int frameCounter = 0;
    while (1) {
        // wait for new image
        if (!sem_wait(newDataSemAddr)) { /* sem_wait returns 0 on success */
            printf("New image received\n\r");

            // reserve a buffer for writing
            struct CAB_BUFFER *cab = (struct CAB_BUFFER *) reserve();
            printf("Buffer reserved\n\r");
            printf("%p\n", cab);

            // save image to buffer
            putmes(cab, shMemPtr, shMemSize);
            printf("Image saved to buffer\n\r");

            // increment frame counter
            frameCounter++;

            // check which tasks should be executed
            callTasks( frameCounter);
        } else {
			printf("[imageViewer] Error on sem_wait\n\r");
		}
    }

    /* Release resources and terminate */
  	printf("releasing resources and ending \n\r");

    closeCab();

  	return SUCCESS;
}

void display_image(void){
    struct CAB_BUFFER * c = getmes();
    printf("GetMes\n");
    unsigned char *pixels = malloc(MAX_WIDTH*MAX_HEIGHT*IMGBYTESPERPIXEL);
    for (int i = 0; i < 100; i++){
        printf("%uc\n", c->img[i]);
    }
    memcpy(pixels, c->img, width*height*IMGBYTESPERPIXEL);
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(screen_texture, NULL, pixels, height* width * IMGBYTESPERPIXEL );
    SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(renderer);			
}

/* **************************************************
 * callTasks() function
*****************************************************/
void callTasks(int frameCounter) {
    // first we call the tasks that should be executed every frame
    // display image
    printf("Image displayed\n\r");

    if (frameCounter  == 1) {
        pthread_create(&tIdWork[0], NULL, (void *)display_image, NULL);
    }
}
