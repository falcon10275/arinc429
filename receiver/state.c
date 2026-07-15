#include "state.h"

// Initialize the starting cruise values
FlightData fd = {0, 0, 100, 0};
arinc429_word_t log_queue[LOG_QUEUE_SIZE];
int log_head;
int log_tail;

// Initialize the locks
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int keep_running = 1;