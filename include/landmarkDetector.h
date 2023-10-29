#include  "../include/point.h"
void detect_landmark(void);

int imgFindBlueSquare(unsigned char * shMemPtr, int startX, int startY, int width, int height, struct Point *b_s, struct Point *b_e);

int imgFindGreenSquare(unsigned char * shMemPtr, int startX, int startY, int width, int height, struct Point *g_s, struct Point *g_e);
