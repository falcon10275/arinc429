#ifndef STATE_H
#define STATE_H

#include <pthread.h>

#include "../common/arinc_429.h"

typedef struct {
    int altitude;
    int rpm;
    int fuel;
    int bank_angle;
} FlightData;

// Extern declarations mean "these exist somewhere, trust me"
extern FlightData fd;
extern pthread_mutex_t screen_mutex;
extern pthread_mutex_t data_mutex;
extern volatile int keep_running;

#define LOG_QUEUE_SIZE 32

extern arinc429_word_t log_queue[LOG_QUEUE_SIZE];
extern int log_head;
extern int log_tail;

#endif // STATE_H