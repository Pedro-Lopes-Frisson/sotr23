/******************************************************
 * Gonçalo Leal - 98008
 * goncalolealsilva@ua.pt
 * 
 * Pedro Lopes - 97827
 * pdfl@ua.pt
 * 
 */

// void initializeObjDetector(int w, int h);

/**
 * @brief Detect obstacles in the image starting from the center of the image and analyzing the pixels
 * in a spiral way only until 35% of the image is analyzed. We consider that the ground is all of one
 * color and the obstacles are of any other color.
 * 
 */
void detectObstaclesSpiral();

int calculateSpirals(int n);

int isWhite(unsigned char *pixel);

/**
 * @brief Detect obstacles in the image starting from the top left corner of the image and analyzing
 * the pixels in a square way only until 35% of the image is analyzed. We consider that the ground is
 * all of one color and the obstacles are of any other color.
 * 
 */
// void detectObstacles(int width, int height);