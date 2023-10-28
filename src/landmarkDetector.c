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
#include  <stdint.h>

#define MAX_WIDTH 1980
#define MAX_HEIGHT 1080

#include <semaphore.h> // For semaphores

/* SDL includes */
#include <SDL2/SDL.h>
extern int width, height;
extern sem_t landmarkCR;

extern SDL_Event event;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;

void detect_landmark(){
    unsigned char pixels[width * height * IMGBYTESPERPIXEL];
    int* pixCountX = (int *)malloc(width * sizeof(int));
    int* pixCountY = (int *)malloc(height * sizeof(int));
    int i,x,y;
    int in_edge, out_edge;
    int  cm_x, cm_y;
    int  cm_x2, cm_y2;

    while(1){
        if ((sem_wait(&landmarkCR)) != 0) { /* enter monitor */
            perror("Error posting semapore for Landmark detection");  /* save error in errno */
            int status = EXIT_FAILURE;
            pthread_exit(&status);
        }
        struct CAB_BUFFER *c = getmes();
        memcpy(pixels, c->img, width * height * IMGBYTESPERPIXEL);
        unget(c);
		// first find blue square
		if(!imgFindBlueSquare(pixels, width, height, &cm_x, &cm_y )) {
			printf("BlueSquare found at (%3d,%3d)\n", cm_x, cm_y);
		} else {
			printf("BlueSquare not found\n");
			continue;
		}			

		//first find blue square
		if(!imgFindGreenSquare(pixels,cm_x, width, height, &cm_x2, &cm_y2 )) {
			printf("GreenSquare found at (%3d,%3d)\n", cm_x2, cm_y2);
		} else {
			printf("Green not found\n");
			continue;
		}		

		if(cm_x2 > cm_y - 5 || cm_x2 > cm_y + 5 )
			printf("LAHNDMARKDETECTED\n\n");

		SDL_RenderClear(renderer);
		SDL_UpdateTexture(screen_texture, NULL, pixels, width * IMGBYTESPERPIXEL);
		SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
		SDL_RenderPresent(renderer);
    }
}

int imgFindBlueSquare(unsigned char * shMemPtr, int width, int height, int *in_edge, int *out_edge){
	#define FINDBLUE_DBG 	0	// Flag to activate output of image processing debug info 
	
	/* Note: the following settings are strongly dependent on illumination intensity and color, ...*/
	/* 		There are much more robust approaches! */
	#define MAGNITUDE 		1.5 		// minimum ratio between Blue and other colors to be considered blue
	#define PIX_THRESHOLD 	30 	// Minimum number of pixels to be considered an object of interest 
	#define LPF_SAMPLES		4 	// Simple average for filtering - number of samples to average 
	
	/* Variables */
	unsigned char *imgPtr;		/* Pointer to image */
	int i,x,y;					/* Indexes */
	int pixCountX[MAX_WIDTH], pixCountY[MAX_HEIGHT];  	/* Count of pixels by row and column */
	int cm_x, cm_y;			/* Coordinates of obgect edges */ 
	int found_blue = 0;
	
	/* Check image size */
	if(width > MAX_WIDTH || height > MAX_HEIGHT) {
		printf("[imgFindBlueSquare]ERROR: image size exceeds the limits allowed\n\r");
		return -1;
	}
	
	/* Reset counters */
	for(x=0; x<MAX_WIDTH; x++)
		pixCountX[x]=0;
	
	for(y=0; y<MAX_HEIGHT; y++)
		pixCountY[y]=0;
		
	/* Process image - find count of blue pixels in each row */	
	/* Note that in memory images show as follows (in pixels, each pixel BGRF): */
	/* Pixel in a row show up in contiguous memory addresses */
	/*	Start from top-left corner */
	/* (0,0) (1,0) (2,0) (3,0) ... (width-1,0)*/
	/* (0,1) (1,1) (2,1) (3,1) ... */
	/* (0,2) (1,2) (2,2) (3,2) ... */
	/*  ...   ...   ...            */
	/* (0,height-1) (1,height-1) (2, height-1) ... (height-1, width-1)*/
	
	imgPtr = shMemPtr;
	for (y = 0; y < height; ++y) {
		if(FINDBLUE_DBG) printf("\n");
		for (x = 0; x < width; x++) {	
			if(FINDBLUE_DBG) {
				if(x < 20) {
					printf("%x:%x:%x - ",*imgPtr, *(imgPtr+1), *(imgPtr+2));
				}			}
			/* Remember that for each pix the access is B+G+R+filler */
			/* Simple approach: intensity of one componet much greater than the other two */
			/* There are much robust approaches ... */
			// imgPtr -> Blue 
			// imgPtr + 1 -> green
			// imgPtr + 2 -> red
			if(*imgPtr > (MAGNITUDE * (*(imgPtr+1))) && *imgPtr > (MAGNITUDE * (*(imgPtr+2))))
				pixCountY[y]+=1;
			
			/* Step to next pixel */
			imgPtr+=4;
		}		
	}
		
	/* Process image - find count of blue pixels in each column */	
	for (x = 0; x < width; x++) {	
		imgPtr = shMemPtr + x * 4; // Offset to the xth column in the firts row
		for (y = 0; y < height; y++) {		
			if(*imgPtr > (MAGNITUDE * (*(imgPtr+1))) && *imgPtr > (MAGNITUDE * (*(imgPtr+2))))
				pixCountX[x]+=1;

			
			/* Step to teh same pixel i the next row */
			imgPtr+=4*width;
		}		
	}
	
	if(FINDBLUE_DBG) {
		printf("\n Image processing results - raw - by row :");
		for(x=0; x<MAX_WIDTH; x++)
			printf("%5d ",pixCountX[x]);
		printf("\n---------------------------\n");
		
		printf("\n Image processing results - raw - by column :");
		for(y=0; y<MAX_HEIGHT; y++)
			printf("%5d ",pixCountY[y]);
		printf("\n---------------------------\n");
	}
	
	/* Apply a simple averaging filter to pixe count */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES); x++) {
		for(i = 1; i < LPF_SAMPLES; i++) {
			pixCountX[x] += pixCountX[x+i];
		}
		pixCountX[x] = pixCountX[x] / LPF_SAMPLES; 
	}
	
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES); y++) {
		for(i = 1; i < LPF_SAMPLES; i++) {
			pixCountY[y] += pixCountY[y+i];
		}
		pixCountY[y] = pixCountY[y] / LPF_SAMPLES; 
	}
	
	if(FINDBLUE_DBG) {
		printf("\n Image processing results - after filtering - by row:");
		for(x=0; x < (MAX_WIDTH - LPF_SAMPLES); x++)
			printf("%5d ",pixCountX[x]);
		printf("\n---------------------------\n");
		
		printf("\n Image processing results - after filtering - by columnn:");
		for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES); y++)
			printf("%5d ",pixCountY[y]);
		printf("\n---------------------------\n");
	}
		
		
	/* Detect Center of Mass (just one object) */
	cm_x = -1; 	// By default not found
	cm_y = -1;		
		
	/* Detect YY CoM */
	*in_edge = -1;	// By default not found
	*out_edge= -1;
	
	/* Detect rising edge - beginning */
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES -1); y++) {
		if((pixCountY[y] < PIX_THRESHOLD) && ((pixCountY[y+1] >= PIX_THRESHOLD))) {
			*in_edge = y;
			break;
		}
	}
	/* Detect falling edge - ending */
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES -1); y++) {
		if((pixCountY[y] >= PIX_THRESHOLD) && ((pixCountY[y+1] < PIX_THRESHOLD))) {
			*out_edge = y;
			break;
		}
	}	
			
	/* Process edges to determine center of mass existence and position */ 		
	/* If object in the left image edge */
	if(*out_edge > 0 && *in_edge == -1)
		*in_edge = 0;
	
	if((*in_edge >= 0) && (*out_edge >= 0))
		cm_y = (*out_edge-*in_edge)/2+*in_edge;
		
	if(FINDBLUE_DBG) {
		if(cm_y >= 0) {
			printf("Blue square center of mass y = %d\n", cm_y);
		} else {
			printf("Blue square center of mass y = NF\n");
		}
	}				
	printf("Start Y: %d\n", *in_edge  );
	printf("end Y: %d\n", *out_edge );
	
		
	/* Detect XX CoM */
	*in_edge = -1;	// By default not found
	*out_edge= -1;
	
	/* Detect rising edge - beginning */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES -1); x++) {
		if((pixCountX[x] < PIX_THRESHOLD) && ((pixCountX[x+1] >= PIX_THRESHOLD))) {
			*in_edge = x;
			break;
		}
	}
	/* Detect falling edge - ending */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES -1); x++) {
		if((pixCountX[x] >= PIX_THRESHOLD) && ((pixCountX[x+1] < PIX_THRESHOLD))) {
			*out_edge = x;
			break;
		}
	}	
			
	/* Process edges to determine center of mass existence and position */ 		
	/* If object in the left image edge */
	if(*out_edge > 0 && *in_edge == -1)
		*in_edge = 0;
	
	if((*in_edge >= 0) && (*out_edge >= 0))
		cm_x = (*out_edge-*in_edge)/2+*in_edge;
		
	if(FINDBLUE_DBG) {
		if(cm_x >= 0) {
			printf("Blue square center of mass x = %d\n", cm_x);
		} else {
			printf("Blue square center of mass x = NF\n");
		}
	}					
		
	printf("Start X: %d\n", *in_edge );
	printf("end X: %d\n", *out_edge );
	/* Return with suitable error code */
	
	if(cm_x >= 0 && cm_y >= 0)
		return 0;	// Success
	else
		return -1; // No objecty
}


int imgFindGreenSquare(unsigned char * shMemPtr, int startX, int width, int height, int *in_edge, int *out_edge){
	#define FINDBLUE_DBG 	0	// Flag to activate output of image processing debug info 
	
	/* Note: the following settings are strongly dependent on illumination intensity and color, ...*/
	/* 		There are much more robust approaches! */
	#define MAGNITUDE 		1.5 		// minimum ratio between Blue and other colors to be considered blue
	#define PIX_THRESHOLD 	30 	// Minimum number of pixels to be considered an object of interest 
	#define LPF_SAMPLES		4 	// Simple average for filtering - number of samples to average 
	
	/* Variables */
	unsigned char *imgPtr;		/* Pointer to image */
	int i,x,y;					/* Indexes */
	int pixCountX[MAX_WIDTH], pixCountY[MAX_HEIGHT];  	/* Count of pixels by row and column */
	int cm_x, cm_y;			/* Coordinates of obgect edges */ 
	int found_blue = 0;
	
	/* Check image size */
	if(width > MAX_WIDTH || height > MAX_HEIGHT) {
		printf("[imgFindBlueSquare]ERROR: image size exceeds the limits allowed\n\r");
		return -1;
	}
	if(startX > width - PIX_THRESHOLD)
		return -1;
	
	/* Reset counters */
	for(x=0; x<MAX_WIDTH; x++)
		pixCountX[x]=0;
	
	for(y=0; y<MAX_HEIGHT; y++)
		pixCountY[y]=0;
		
	/* Process image - find count of blue pixels in each row */	
	/* Note that in memory images show as follows (in pixels, each pixel BGRF): */
	/* Pixel in a row show up in contiguous memory addresses */
	/*	Start from top-left corner */
	/* (0,0) (1,0) (2,0) (3,0) ... (width-1,0)*/
	/* (0,1) (1,1) (2,1) (3,1) ... */
	/* (0,2) (1,2) (2,2) (3,2) ... */
	/*  ...   ...   ...            */
	/* (0,height-1) (1,height-1) (2, height-1) ... (height-1, width-1)*/
	
	imgPtr = shMemPtr;
	for (y = 0; y < height; ++y) {
		if(FINDBLUE_DBG) printf("\n");
		for (x = startX; x < width; x++) {	
			if(FINDBLUE_DBG) {
				if(x < 20) {
					printf("%x:%x:%x - ",*imgPtr, *(imgPtr+1), *(imgPtr+2));
				}			}
			/* Remember that for each pix the access is B+G+R+filler */
			/* Simple approach: intensity of one componet much greater than the other two */
			/* There are much robust approaches ... */
			// imgPtr -> Blue 
			// imgPtr + 1 -> green
			// imgPtr + 2 -> red
			if(*(imgPtr+1) > (MAGNITUDE * (*imgPtr)) && *(imgPtr+1) > (MAGNITUDE * (*(imgPtr+2))))
				pixCountY[y]+=1;
			
			/* Step to next pixel */
			imgPtr+=4;
		}		
	}
		
	/* Process image - find count of blue pixels in each column */	
	for (x = startX; x < width; x++) {	
		imgPtr = shMemPtr + x * 4; // Offset to the xth column in the firts row
		for (y = 0; y < height; y++) {		
			if(*(imgPtr+1) > (MAGNITUDE * (*imgPtr)) && *(imgPtr+1) > (MAGNITUDE * (*(imgPtr+2))))
				pixCountX[x]+=1;

			
			/* Step to teh same pixel i the next row */
			imgPtr+=4*width;
		}		
	}
	
	if(FINDBLUE_DBG) {
		printf("\n Image processing results - raw - by row :");
		for(x=startX; x<MAX_WIDTH; x++)
			printf("%5d ",pixCountX[x]);
		printf("\n---------------------------\n");
		
		printf("\n Image processing results - raw - by column :");
		for(y=0; y<MAX_HEIGHT; y++)
			printf("%5d ",pixCountY[y]);
		printf("\n---------------------------\n");
	}
	
	/* Apply a simple averaging filter to pixe count */
	for(x=startX; x < (MAX_WIDTH - LPF_SAMPLES); x++) {
		for(i = 1; i < LPF_SAMPLES; i++) {
			pixCountX[x] += pixCountX[x+i];
		}
		pixCountX[x] = pixCountX[x] / LPF_SAMPLES; 
	}
	
	for(y=startX; y < (MAX_HEIGHT - LPF_SAMPLES); y++) {
		for(i = 1; i < LPF_SAMPLES; i++) {
			pixCountY[y] += pixCountY[y+i];
		}
		pixCountY[y] = pixCountY[y] / LPF_SAMPLES; 
	}
	
	if(FINDBLUE_DBG) {
		printf("\n Image processing results - after filtering - by row:");
		for(x=0; x < (MAX_WIDTH - LPF_SAMPLES); x++)
			printf("%5d ",pixCountX[x]);
		printf("\n---------------------------\n");
		
		printf("\n Image processing results - after filtering - by columnn:");
		for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES); y++)
			printf("%5d ",pixCountY[y]);
		printf("\n---------------------------\n");
	}
		
		
	/* Detect Center of Mass (just one object) */
	cm_x = -1; 	// By default not found
	cm_y = -1;		
		
	/* Detect YY CoM */
	*in_edge = -1;	// By default not found
	*out_edge= -1;
	
	/* Detect rising edge - beginning */
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES -1); y++) {
		if((pixCountY[y] < PIX_THRESHOLD) && ((pixCountY[y+1] >= PIX_THRESHOLD))) {
			*in_edge = y;
			break;
		}
	}
	/* Detect falling edge - ending */
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES -1); y++) {
		if((pixCountY[y] >= PIX_THRESHOLD) && ((pixCountY[y+1] < PIX_THRESHOLD))) {
			*out_edge = y;
			break;
		}
	}	
			
	/* Process edges to determine center of mass existence and position */ 		
	/* If object in the left image edge */
	if(*out_edge > 0 && *in_edge == -1)
		*in_edge = 0;
	
	if((*in_edge >= 0) && (*out_edge >= 0))
		cm_y = (*out_edge-*in_edge)/2+*in_edge;
		
	if(FINDBLUE_DBG) {
		if(cm_y >= 0) {
			printf("Blue square center of mass y = %d\n", cm_y);
		} else {
			printf("Blue square center of mass y = NF\n");
		}
	}				
	printf("Start Y: %d\n", *in_edge  );
	printf("end Y: %d\n", *out_edge );
	
		
	/* Detect XX CoM */
	*in_edge = -1;	// By default not found
	*out_edge= -1;
	
	/* Detect rising edge - beginning */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES -1); x++) {
		if((pixCountX[x] < PIX_THRESHOLD) && ((pixCountX[x+1] >= PIX_THRESHOLD))) {
			*in_edge = x;
			break;
		}
	}
	/* Detect falling edge - ending */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES -1); x++) {
		if((pixCountX[x] >= PIX_THRESHOLD) && ((pixCountX[x+1] < PIX_THRESHOLD))) {
			*out_edge = x;
			break;
		}
	}	
			
	/* Process edges to determine center of mass existence and position */ 		
	/* If object in the left image edge */
	if(*out_edge > 0 && *in_edge == -1)
		*in_edge = 0;
	
	if((*in_edge >= 0) && (*out_edge >= 0))
		cm_x = (*out_edge-*in_edge)/2+*in_edge;
		
	if(FINDBLUE_DBG) {
		if(cm_x >= 0) {
			printf("Blue square center of mass x = %d\n", cm_x);
		} else {
			printf("Blue square center of mass x = NF\n");
		}
	}					
		
	printf("Start X: %d\n", *in_edge );
	printf("end X: %d\n", *out_edge );
	/* Return with suitable error code */
	
	if(cm_x >= 0 && cm_y >= 0)
		return 0;	// Success
	else
		return -1; // No objecty
}


