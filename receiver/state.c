#include "state.h"

// Initialize the starting cruise values
FlightData fd = {10000, 2500, 100.0, 0.0};

// Initialize the locks
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int keep_running = 1;