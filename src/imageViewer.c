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

/* Custom includes */
#include "../include/cab.h" // For cab struct
#include "../include/imageViewer.h" // For sdl image viewer

struct IMAGE_DISPLAY *initDisplayer(int height, int width, int btspp, char *appName) {
    struct IMAGE_DISPLAY *display = malloc(sizeof(struct IMAGE_DISPLAY));
    display->height = height;
    display->width = width;
    display->btspp = btspp;
    display->appName = appName;

    SDL_Init(SDL_INIT_VIDEO);

    display->window = SDL_CreateWindow(
        appName, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width,
        height, SDL_WINDOW_RESIZABLE
    );

    display->renderer = SDL_CreateRenderer(
        display->window, -1, SDL_RENDERER_PRESENTVSYNC
    );

    /* Limit the window size so that it cannot */
	/* be smaller than teh webcam image size */
	SDL_SetWindowMinimumSize(display->window, width, height);

	SDL_RenderSetLogicalSize(display->renderer, width, height);
	SDL_RenderSetIntegerScale(display->renderer, 1);

    display->texture = SDL_CreateTexture(
        display->renderer, SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING, width, height
    );

    return display;
}

void displayImage(struct IMAGE_DISPLAY *display, struct CAB_BUFFER *cab) {
    SDL_UpdateTexture(display->texture, NULL, cab->img, display->width * display->btspp);
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}