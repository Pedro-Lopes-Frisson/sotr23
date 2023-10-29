/**
 *		CAB header file
 *		Declaration of access primitives and data structures
 *
 *
 * */

#define N_TASKS 10
#define DIM_X 640
#define DIM_Y 480
#define MAX_CHARS 50
#define IMGBYTESPERPIXEL                                                       \
  4 /* Number of bytes per pixel. Allowed formats are RGB888, RGBA8888 and */

struct CAB_BUFFER {
  struct CAB_BUFFER *next; // pointer to the next buffer
  int use;
  unsigned char *img;
};

/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

static void initialization(void);

/*
 *  create a new CAB with specified id and with n_buffers and it's dimension
 * */
int openCab(const char *cab_name, const int max_buffers, const int dim_x, const int dim_y);

/*
 * destroy cab
 *
 * */
int closeCab(void);

/*
 * reserve a free buffer
 *
 * */
struct CAB_BUFFER *reserve(void);

/*
 * save data to buffer
 *
 * */
void putmes(struct CAB_BUFFER *c, unsigned char *data, const int size);

/*
 * get data from buffer
 *
 * */
struct CAB_BUFFER *getmes(void);

/*
 *	release a buffer
 * **/
void unget(struct CAB_BUFFER *buffer);
