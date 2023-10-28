/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 * 
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 * 
 * Responsible for displaying the image in a window.
 */

/* SDL includes */
#include <SDL2/SDL.h>

/* General includes */
#include <semaphore.h> /* for semaphores */
#include <pthread.h> /* for threads */

/* Custom includes */
#include "../include/cab.h" // For cab struct
#include "../include/imageViewer.h" // For sdl image viewer

extern int width, height;
extern sem_t displayImageCR;

extern SDL_Event event;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;

void display_image() {
    while(1) {
        if ((sem_wait(&displayImageCR)) != 0) {
            perror("Error posting semapore for image display");  /* save error in errno */
            int status = EXIT_FAILURE;
            pthread_exit(&status);
        }

        struct CAB_BUFFER *c = getmes();
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, c->img, width * IMGBYTESPERPIXEL );
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        unget(c);
    }
}