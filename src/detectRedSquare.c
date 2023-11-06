
#include  "../include/point.h"
#include  "../include/cab.h"
#include  "../include/detectRedSquare.h"
#include  "../include/object.h"

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <pthread.h>
#include  <stdint.h>

#define MAX_WIDTH 1980
#define MAX_HEIGHT 1080

#include <semaphore.h> // For semaphores
		       //
#include "../include/varsDisplayer.h" // 

/* SDL includes */
#include <SDL2/SDL.h>

#define FINDBLUE_DBG 	0	// Flag to activate output of image processing debug info 

/* Note: the following settings are strongly dependent on illumination intensity and color, ...*/
/* 		There are much more robust approaches! */
#define MAGNITUDE 		1.5 		// minimum ratio between Blue and other colors to be considered blue
#define PIX_THRESHOLD 	30 	// Minimum number of pixels to be considered an object of interest 
#define LPF_SAMPLES		4 	// Simple average for filtering - number of samples to average 


extern int width, height;
extern sem_t redCR;

extern SDL_Event event;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;

extern char varDispShMemActiveFlag;
extern char varDispSemActiveFlag;
extern void *varDispShMemPtr;
extern sem_t *varDispSemAddr;

static int pixCountX[MAX_WIDTH];
static int pixCountY[MAX_HEIGHT];
static int i,x,y;					/* Indexes */
static int cm_x, cm_y;			/* Coordinates of obgect edges */ 
static int in_edge, out_edge;
static struct Point b_s, b_e; // blue square edges

void detect_red_square(){
    unsigned char pixels[width * height * IMGBYTESPERPIXEL];


    while(1){
        if ((sem_wait(&redCR)) != 0) { /* enter monitor */
            perror("Error posting semapore for Landmark detection");  /* save error in errno */
            int status = EXIT_FAILURE;
            pthread_exit(&status);
        }
        struct CAB_BUFFER *c = getmes();
        memcpy(pixels, c->img, width * height * IMGBYTESPERPIXEL);
        unget(c);
	// first find blue square
	if(!imgFindRedSquare(pixels, 0, 0, width, height, &b_s, &b_e )) {
		// found_object(b_s.x + b_e.x / 2 + b_s.x, b_s.y + b_e.y / 2 + b_s.y);
		// TODO: send to shmem

		if (varDispShMemActiveFlag) {
			// create object
			struct detected_obj obj;

			obj.cm_x = cm_x;
			obj.cm_y = cm_y;
			memcpy(obj.obj_name, "square", 7);

			// copy to shmem
			memcpy(varDispShMemPtr, &obj, sizeof(struct detected_obj));
		}

		if (varDispSemActiveFlag) {
			// post semaphore
			if ((sem_post(varDispSemAddr)) != 0) {
			perror("Error posting semapore for image display"); /* save error in errno */
			int status = EXIT_FAILURE;
			pthread_exit(&status);
			}
		}

	} else {
		printf("RedSquare not found\n");
	}			
    }
}


int imgFindRedSquare(unsigned char * shMemPtr, int startX, int startY, int width, int height, struct Point *b_s, struct Point *b_e){
	
	/* Variables */
	unsigned char *imgPtr;		/* Pointer to image */
	/* Check image size */
	if(width > MAX_WIDTH || height > MAX_HEIGHT) {
		printf("[imgFindBlueSquare]ERROR: image size exceeds the limits allowed\n\r");
		return -1;
	}

	if(startX > width - PIX_THRESHOLD)
		return -1;

	if(startY > height - PIX_THRESHOLD)
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
	for (y = startY; y < height; ++y) {
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
			if(*(imgPtr+2) > (MAGNITUDE * (*(imgPtr))) && *(imgPtr + 2) > (MAGNITUDE * (*(imgPtr+1))))
				pixCountY[y]+=1;
			
			/* Step to next pixel */
			imgPtr+=4;
		}		
	}
		
	/* Process image - find count of blue pixels in each column */	
	for (x = startX; x < width; x++) {	
		imgPtr = shMemPtr + x * 4; // Offset to the xth column in the firts row
		for (y = startY; y < height; y++) {		
			if(*(imgPtr+2) > (MAGNITUDE * (*(imgPtr))) && *(imgPtr + 2) > (MAGNITUDE * (*(imgPtr+1))))
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
	in_edge = -1;	// By default not found
	out_edge= -1;
	
	/* Detect rising edge - beginning */
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES -1); y++) {
		if((pixCountY[y] < PIX_THRESHOLD) && ((pixCountY[y+1] >= PIX_THRESHOLD))) {
			in_edge = y;
			break;
		}
	}
	/* Detect falling edge - ending */
	for(y=0; y < (MAX_HEIGHT - LPF_SAMPLES -1); y++) {
		if((pixCountY[y] >= PIX_THRESHOLD) && ((pixCountY[y+1] < PIX_THRESHOLD))) {
			out_edge = y;
			break;
		}
	}	
			
	/* Process edges to determine center of mass existence and position */ 		
	/* If object in the left image edge */
	if(out_edge > 0 && in_edge == -1)
		in_edge = 0;
	
	if((in_edge >= 0) && (out_edge >= 0))
		cm_y = (out_edge-in_edge)/2+in_edge;
		
	if(FINDBLUE_DBG) {
		if(cm_y >= 0) {
			printf("Blue square center of mass y = %d\n", cm_y);
		} else {
			printf("Blue square center of mass y = NF\n");
		}
	}				

	b_s->y = in_edge;
	b_e->y = out_edge;
	printf("Start Y: %d\n", in_edge  );
	printf("end Y: %d\n", out_edge );
	
		
	/* Detect XX CoM */
	in_edge = -1;	// By default not found
	out_edge= -1;
	
	/* Detect rising edge - beginning */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES -1); x++) {
		if((pixCountX[x] < PIX_THRESHOLD) && ((pixCountX[x+1] >= PIX_THRESHOLD))) {
			in_edge = x;
			break;
		}
	}
	/* Detect falling edge - ending */
	for(x=0; x < (MAX_WIDTH - LPF_SAMPLES -1); x++) {
		if((pixCountX[x] >= PIX_THRESHOLD) && ((pixCountX[x+1] < PIX_THRESHOLD))) {
			out_edge = x;
			break;
		}
	}	
			
	/* Process edges to determine center of mass existence and position */ 		
	/* If object in the left image edge */
	if(out_edge > 0 && in_edge == -1)
		in_edge = 0;
	
	if((in_edge >= 0) && (out_edge >= 0))
		cm_x = (out_edge-in_edge)/2+in_edge;
		
	if(FINDBLUE_DBG) {
		if(cm_x >= 0) {
			printf("Blue square center of mass x = %d\n", cm_x);
		} else {
			printf("Blue square center of mass x = NF\n");
		}
	}					
	b_s->x = in_edge;
	b_e->x = out_edge;
	printf("Start X: %d\n", in_edge );
	printf("end X: %d\n", out_edge );
	/* Return with suitable error code */
	
	if(cm_x >= 0 && cm_y >= 0)
		return 0;	// Success
	else
		return -1; // No objecty
}
