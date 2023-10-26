/*
 *	Code to execute in order to identify landmarks in images
 *	this landmarks should be a identified as chess pattern retangle
 * */

#include  "../include/landmarkDetector.h"
#include  "../include/cab.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <pthread.h>

#define MAX_WIDTH 1980
#define MAX_HEIGHT 1080

/* SDL includes */
#include <SDL2/SDL.h>

#include <semaphore.h> // For semaphores

static unsigned char pixels[MAX_WIDTH * MAX_HEIGHT * IMGBYTESPERPIXEL];
extern int width, height;
extern sem_t landmarkCR;

extern SDL_Event event;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;

void detect_landmark(){
    while(1){
        if ((sem_wait(&landmarkCR)) != 0) { /* enter monitor */
            perror("Error posting semapore for Landmark detection");  /* save error in errno */
            int status = EXIT_FAILURE;
            pthread_exit(&status);
        }
        struct CAB_BUFFER *c = getmes();
        printf("||||||||GetMes %p %d %d|||||||\n", c, width, height);
        memcpy(pixels, c->img, width * height * IMGBYTESPERPIXEL);
        unget(c);
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, width * IMGBYTESPERPIXEL);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
