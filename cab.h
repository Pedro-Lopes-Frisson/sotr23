/**
 *		CAB header file
 *		Declaration of access primitives and data structures
 *
 *
 * */
#include <bool.h>

static int max_buff;
static int dim_buff;
static struct CAB_BUFFER *mrb; // most recent buffer
static char cab_id[50]

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;


struct CAB_BUFFER{
	struct CAB_BUFFER *next; // pointer to the next buffer
	int use = 0;
	unsigned char **data;
};
 
/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

void initialization(void);

/*
 *  create a new CAB with specified id and with n_buffers and it's dimension
 * */
bool open(const char *cab_name, const int max_buffers, const int dim_buff);


/*
 * destroy cab
 *
 * */
bool close(void);

/*
 * reserve a free buffer
 *
 * */
struct CAB_BUFFER * reserve(void);

/*
 * save data to buffer
 *
 * */
void * putmes(struct CAB_BUFFER * c, unsigned char *data);

/*
 * get data from buffer
 *
 * */
void * getmes(void);

/*
 *	release a buffer
 * **/
void unget(struct CAB_BUFFER *buffer);
