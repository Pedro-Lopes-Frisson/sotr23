
struct Point {
	int x;
	int y;
};
void detect_landmark(void);

int imgFindBlueSquare(unsigned char * shMemPtr, int width, int height, int *in_edge, int *out_edge);
int imgFindGreenSquare(unsigned char * shMemPtr, int startX, int width, int height, int *in_edge, int *out_edge);
