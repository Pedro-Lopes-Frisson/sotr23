/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 *
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 *
 * Responsible for all the image analysis tasks such as obstacles detection,
 * object detection, etc.
 */

/* General includes */
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>   /* for threads */
#include <semaphore.h> /* for semaphores */

#include "../include/cab.h"
#include "../include/objDetector.h"
#include "../include/object.h"

/* Global settings */
#define FALSE 0 /* The usual true and false */
#define TRUE (!0)
#define SUCCESS 0          /* Program terminates normally */
#define IMGBYTESPERPIXEL 4 /* Number of bytes per pixel. */
#define EXIT_FAILURE 1

extern int width, height;
extern sem_t detectObstaclesCR;

extern char varDispShMemActiveFlag;
extern char varDispSemActiveFlag;
extern void *varDispShMemPtr;
extern sem_t *varDispSemAddr;

void detect_obstacles_spiral() {

/* Let's consider that the ground is white and the obstacles are of a different
 * color */

// Now, let's analyze the image
#define MAGNITUDE                                                              \
  1.5 // minimum ratio between ground and other colors to be considered
      // different
#define PIX_THRESHOLD                                                          \
  30 // Minimum number of pixels to be considered an object of interest
#define LPF_SAMPLES                                                            \
  4 // Simple average for filtering - number of samples to average
#define PERCENTAGE 0.35 // Percentage of the image to be analyzed

  int pixCountX[width], pixCountY[height]; /* Number of pixels of a different
                                              color in each column and row */
  int in_edge, out_edge; /* coordinates of the edges of the object */

  while (1) {
    if ((sem_wait(&detectObstaclesCR)) != 0) {
      perror(
          "Error posting semapore for image display"); /* save error in errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }

    /* First, let's get the image from the cab */
    struct CAB_BUFFER *c = getmes();

    /* Reset counters */
    for (int i = 0; i < width; i++)
      pixCountX[i] = 0;

    for (int i = 0; i < height; i++)
      pixCountY[i] = 0;

    /* Count the number of pixels of a different color in each column and row */
    /* starting from the center of the image and analyzing the pixels in a
     * spiral way */
    /* only until 35% of the image is analyzed */
    int centerCol = width / 2, centerRow = height / 2; /* Center of the image */
    int row, col; /* Current row and column */

    int spirals = calculateSpirals(
        PERCENTAGE * (width * height)); /* Number of spirals to be done */

    /* Define the direction vectors for right, down, left, and up */
    /* we go right (col++) -> down (row++) -> left (col--) -> up (row--) */
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};

    unsigned char *imgPtr;
    for (int spiral = 0; spiral < spirals; spiral++) {
      row = centerRow - spiral;
      col = centerCol - spiral;

      imgPtr = c->img + (row * width + col) * IMGBYTESPERPIXEL;

      for (int direction = 0; direction < 4; direction++) {
        while (row > -1 && col > -1 && row < height && col < width &&
               row >= centerRow - spiral && row <= centerRow + spiral &&
               col >= centerCol - spiral && col <= centerCol + spiral) {
          if (isWhite(imgPtr) == FALSE) {
            pixCountX[col]++;
            pixCountY[row]++;
          }

          /* move to next pixel */
          row += dr[direction];
          col += dc[direction];

          imgPtr = c->img + (row * width + col) * IMGBYTESPERPIXEL;

          /* if we are in the initial position, we are done */
          if (col == centerCol - spiral && row == centerRow - spiral) {
            break;
          }
        }

        /* move to the next direction */
        switch (direction) {
        case 0:
          /* we will go down */
          col = centerCol + spiral;
          row = centerRow - spiral;
          break;
        case 1:
          /* we will go left */
          col = centerCol + spiral;
          row = centerRow + spiral;
          break;
        case 2:
          /* we will go up */
          col = centerCol - spiral;
          row = centerRow + spiral;
          break;
        }

        imgPtr = c->img + (row * width + col) * IMGBYTESPERPIXEL;

        /* We can't move in any direction */
        if (spiral == 0) {
          break;
        }
      }
    }

    /* Apply a simple averaging filter to pixe count */
    for (int x = 0; x < (width - LPF_SAMPLES); x++) {
      for (int i = 1; i < LPF_SAMPLES; i++) {
        pixCountX[x] += pixCountX[x + i];
      }
      pixCountX[x] = pixCountX[x] / LPF_SAMPLES;
    }

    for (int y = 0; y < (height - LPF_SAMPLES); y++) {
      for (int i = 1; i < LPF_SAMPLES; i++) {
        pixCountY[y] += pixCountY[y + i];
      }
      pixCountY[y] = pixCountY[y] / LPF_SAMPLES;
    }

    /* Detect Center of Mass (just one object) */
    int16_t cm_x = -1; // By default not found
    int16_t cm_y = -1;

    /* Detect YY CoM */
    in_edge = -1; // By default not found
    out_edge = -1;

    /* Detect rising edge - beginning */
    for (int y = 0; y < (height - LPF_SAMPLES - 1); y++) {
      if ((pixCountY[y] < PIX_THRESHOLD) &&
          ((pixCountY[y + 1] >= PIX_THRESHOLD))) {
        in_edge = y;
        break;
      }
    }
    /* Detect falling edge - ending */
    for (int y = 0; y < (height - LPF_SAMPLES - 1); y++) {
      if ((pixCountY[y] >= PIX_THRESHOLD) &&
          ((pixCountY[y + 1] < PIX_THRESHOLD))) {
        out_edge = y;
        break;
      }
    }

    /* Process edges to determine center of mass existence and position */
    /* If object in the left image edge */
    if (out_edge > 0 && in_edge == -1)
      in_edge = 0;

    if ((in_edge >= 0) && (out_edge >= 0))
      cm_y = (out_edge - in_edge) / 2 + in_edge;

    /* Detect XX CoM */
    in_edge = -1; // By default not found
    out_edge = -1;

    /* Detect rising edge - beginning */
    for (int x = 0; x < (width - LPF_SAMPLES - 1); x++) {
      if ((pixCountX[x] < PIX_THRESHOLD) &&
          ((pixCountX[x + 1] >= PIX_THRESHOLD))) {
        in_edge = x;
        break;
      }
    }
    /* Detect falling edge - ending */
    for (int x = 0; x < (width - LPF_SAMPLES - 1); x++) {
      if ((pixCountX[x] >= PIX_THRESHOLD) &&
          ((pixCountX[x + 1] < PIX_THRESHOLD))) {
        out_edge = x;
        break;
      }
    }

    /* Process edges to determine center of mass existence and position */
    /* If object in the left image edge */
    if (out_edge > 0 && in_edge == -1)
      in_edge = 0;

    if ((in_edge >= 0) && (out_edge >= 0))
      cm_x = (out_edge - in_edge) / 2 + in_edge;

    if (cm_x >= 0 && cm_y >= 0) {
	    // found_ball(cm_x, cm_y);
      // TODO: send to shmem

      if (varDispShMemActiveFlag) {
        // create object
        struct detected_obj obj;

        obj.cm_x = cm_x;
        obj.cm_y = cm_y;
        memcpy(obj.obj_name, "ball", 4);

        // copy to shmem
	memcpy(&((struct detected_obj *)varDispShMemPtr)[1], &obj, sizeof(struct detected_obj));
      }

      if (varDispSemActiveFlag) {
        // post semaphore
        if ((sem_post(varDispSemAddr)) != 0) {
          perror("Error posting semapore for image display"); /* save error in errno */
          int status = EXIT_FAILURE;
          pthread_exit(&status);
        }
      }

      printf("ball at (%d,%d)\n\r", cm_x, cm_y);
    }
    else
      printf("objectOOOOOOOOOOOOOO not found\n\r"); // No object found

    /* Free the cab */
    unget(c);
  }

}

int calculateSpirals(int n) {
  int i = 1;
  int sum = 1;
  while (sum < n) {
    sum += 8 * i;
    i++;
  }
  return i;
}

int isWhite(unsigned char *pixel) {
  if (pixel[0] >= 200 && pixel[1] >= 200 && pixel[2] >= 200) {
    return TRUE;
  }
  return FALSE;
}
