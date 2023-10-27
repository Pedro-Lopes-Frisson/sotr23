/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 * 
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 * 
 */

/* SDL includes */
#include <SDL2/SDL.h>

#define MAX_WIDTH	1980	/* Sets the max allowed image width */
#define MAX_HEIGHT	1024	/* Sets the max allowed image height */

struct IMAGE_DISPLAY {
    SDL_Window *window;
    int height;
    int width;
    int btspp; /* bytes per pixel */
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    unsigned char *appName;
};

/**
 * @brief initialize the image displayer
 * 
 * @param height 
 * @param width 
 * @param btspp 
 */
void initDisplayer(int height, int width, int btspp, char *appName);

/**
 * @brief display the image
 */
void displayImage();